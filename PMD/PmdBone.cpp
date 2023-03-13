#include "pch.h"
#include "PmdBone.h"

using namespace::DirectX;

PmdBone::PmdBone()
{
}

//ボーンテーブル、回転行列、IK情報、モーション情報を初期化する
PmdBone::PmdBone(std::vector<PmdBoneData> pmdBoneDatas, std::vector<PMDIK> pmdIks, std::string motionPath)
{
	CreateBoneNodeTable(pmdBoneDatas);
	InitBoneMatrices(pmdBoneDatas);

	LoadPmdIks(pmdIks);
	vmdData = VMDData(motionPath);
}

//親子関係のあるボーンノードテーブルを作る
void PmdBone::CreateBoneNodeTable(std::vector<PmdBoneData> pmdBoneDatas)
{
	boneNames.resize(pmdBoneDatas.size());
	boneNodeAddressArray.resize(pmdBoneDatas.size());

	//ボーンネームズと連想配列とノードアレイに全ボーンデータを入れる
	for (int idx = 0; idx < pmdBoneDatas.size(); ++idx)
	{
		auto& pmdbonedata = pmdBoneDatas[idx];
		boneNames[idx] = pmdbonedata.boneName;

		auto& node = boneNodeTable[pmdbonedata.boneName];
		node.boneIdx = idx;
		node.startPos = pmdbonedata.pos;
		node.boneType = pmdbonedata.Type;
		node.ikParentBone = pmdbonedata.ikBoneNo;

		boneNodeAddressArray[idx] = &node;
	}

	//全ての親子関係を作る
	for (auto& pmdbonedata : pmdBoneDatas)
	{
		//早期continue
		if (pmdbonedata.parentNo >= pmdBoneDatas.size())
		{
			continue;
		}

		//イテレータが指すノードの親ノードを格納し、それを使い親ノードの子メンバに自分を追加する。
		//親:子の関係は1:1-nとなり、逆はありえない
		auto parentName = boneNames[pmdbonedata.parentNo];
		boneNodeTable[parentName].children.emplace_back(&boneNodeTable[pmdbonedata.boneName]);
	}

}

void PmdBone::LoadPmdIks(std::vector<PMDIK> pmdIk)
{
	motionIKs.resize(pmdIk.size());

	for (int i = 0; i < pmdIk.size(); ++i)
	{
		motionIKs[i] = pmdIk[i];
	}
}

//ボーン変換行列初期化
void PmdBone::InitBoneMatrices(std::vector<PmdBoneData> pmdBoneDatas)
{
	boneMatrices.resize(pmdBoneDatas.size());

	std::fill(boneMatrices.begin(), boneMatrices.end(), XMMatrixIdentity());


}

//フレームを見てキーフレームを発見、前キーフレームと補間し回転行列を決定する。
//TODO: この関数は分割したい。
void PmdBone::SetBoneMatrices(unsigned int frameNo)
{
	std::fill(boneMatrices.begin(), boneMatrices.end(), XMMatrixIdentity());

	for (auto& bonemotion : vmdData.motionDatas)
	{
		//頂点のボーンデータとモーションのボーンデータが一致することを確認、なければcontinue
		auto itBoneNode = boneNodeTable.find(bonemotion.first);
		if (itBoneNode == boneNodeTable.end())
		{
			continue;
		}

		auto node = boneNodeTable[bonemotion.first];
		auto motions = bonemotion.second;

		//与えられたフレーム番号とモーションのフレーム番号が超えているか判定する関数オブジェクト
		auto predicate = [frameNo](const Motion& motion) {
			return motion.frameNo <= frameNo;
		};

		//近い値のイテレーターを得る、昇順に並んでいるデータを探索するのでリバースイテレーターを使う
		auto rit = std::find_if(
			motions.rbegin(), motions.rend(), predicate
		);

		//モーションデータがないならcontinue
		if (rit == motions.rend())
		{
			continue;
		}

		//前キーフレームと次キーフレームで補間を行う
		XMMATRIX rotation;
		auto it = rit.base();

		if (it == motions.end())
		{
			rotation = XMMatrixRotationQuaternion(rit->quaternion);
		}
		else 
		{
			//ベジェ曲線補間
			auto t = static_cast<float>(frameNo - rit->frameNo)
				/ static_cast<float>(it->frameNo - rit->frameNo);
			t = GetYFromXOnBezier(t, it->controlPoint1, it->controlPoint2, 12);
			rotation = XMMatrixRotationQuaternion(XMQuaternionSlerp(rit->quaternion, it->quaternion, t));
			//rotation = XMMatrixRotationQuaternion(rit->quaternion);
		}

		//得た回転行列を格納。センターから再帰で伝搬させる。
		auto& pos = node.startPos;
		auto matrix = XMMatrixTranslation(-pos.x, -pos.y, -pos.z)
			* rotation
			* XMMatrixTranslation(pos.x, pos.y, pos.z);
		boneMatrices[node.boneIdx] = matrix;
	}

	RecursiveMatrixMultiply(&boneNodeTable["センター"], XMMatrixIdentity());
}


void PmdBone::RecursiveMatrixMultiply(BoneNode* node, const XMMATRIX& mat)
{
	//格納されている単位行列に得た変換行列を掛け、データを保存
	boneMatrices[node->boneIdx] *= mat;

	for (auto& childNode : node->children)
	{
		RecursiveMatrixMultiply(childNode, boneMatrices[node->boneIdx] );
	}
}

float PmdBone::GetYFromXOnBezier(float x, const XMFLOAT2& controlPoint1, const XMFLOAT2& controlPoint2, uint8_t max_steps)
{
	if (controlPoint1.x == controlPoint1.y && controlPoint2.x == controlPoint2.y)
	{
		return x;
	}

	float t = x;
	//係数
	const float k0 = 1 + 3* controlPoint1.x - 3*controlPoint2.x;
	const float k1 = 3 * controlPoint2.x - 6 * controlPoint1.x;
	const float k2 = 3 * controlPoint1.x;

	//判定値
	constexpr float epsilon = 0.0005f;

	//近似値まで計算を回す
	for (int i = 0; i < max_steps; ++i)
	{

		auto ft = k0*t*t*t + k1*t*t + k2*t - x;

		if (ft <= epsilon && ft >= -epsilon)
		{
			break;
		}
		t -= ft / 2;
	}

	auto r = 1 - t;
	auto ret = t*t*t + 3*t*t*r*controlPoint2.y + 3*t*r*r* controlPoint1.y;
	
	return ret;
}

void PmdBone::IKSolve()
{
	for (auto& ik : motionIKs)
	{
		auto childrenNodesCount = ik.nodeIdx.size();

		//CCD-IKは重いので間点が少なければ簡単なIKで解く
		switch (childrenNodesCount)
		{
		case 0: //エラー
			assert(0);
			continue;
		case 1:
			SolveLookAt(ik);
			break;
		case 2:
			SolveCosineIK(ik);
			break;
		default:
			SolveCCDIK(ik);
		}
	}
}

void PmdBone::SolveCCDIK(const PMDIK& ik)
{
}

void PmdBone::SolveCosineIK(const PMDIK& ik)
{
}

void PmdBone::SolveLookAt(const PMDIK& ik)
{
	auto rootNode = boneNodeAddressArray[ik.nodeIdx[0]];
	auto targetNode = boneNodeAddressArray[ik.boneIdx];

	auto rootPos1 = XMLoadFloat3(&rootNode->startPos);
	auto targetPos1 = XMLoadFloat3(&targetNode->startPos);

	auto rootPos2 = XMVector3TransformCoord(rootPos1, boneMatrices[ik.nodeIdx[0]]);
	auto targetPos2 = XMVector3TransformCoord(targetPos1, boneMatrices[ik.boneIdx]);

	auto originVec = XMVectorSubtract(targetPos1, rootPos1);
	auto targetVec = XMVectorSubtract(targetPos2, rootPos2);

	originVec = XMVector3Normalize(originVec);
	targetVec = XMVector3Normalize(targetVec);
	XMFLOAT3 up = XMFLOAT3(0, 1, 0);
	XMFLOAT3 right = XMFLOAT3(1, 0, 0);

	boneMatrices[ik.nodeIdx[0]] = LookAtMatrix(originVec, targetVec, up, right);

}

//二回外積を計算し、座標軸を得てLookAt行列を返す
XMMATRIX PmdBone::LookAtMatrix(const XMVECTOR& lookat, XMFLOAT3& up, XMFLOAT3& right)
{
	//向かせたい方向
	XMVECTOR vz = lookat;

	//仮のy軸ベクトル
	XMVECTOR vy = XMVector3Normalize(XMLoadFloat3(&up));

	//x軸ベクトル
	XMVECTOR vx = XMVector3Normalize(XMVector3Cross(vy, vz));
	//y軸ベクトル
	vy = XMVector3Normalize(XMVector3Cross(vz, vx));

	//lookatとupの方向が同じ場合rightを使い作成する
	if (std::abs(XMVector3Dot(vy, vz).m128_f32[0]) == 1.0f)
	{
		vx = XMVector3Normalize(XMLoadFloat3(&right));
		vy = XMVector3Normalize(XMVector3Cross(vz, vx));
		vx = XMVector3Normalize(XMVector3Cross(vy, vz));
	}

	XMMATRIX ret = XMMatrixIdentity();
	ret.r[0] = vx;
	ret.r[1] = vy;
	ret.r[2] = vz;

	return ret;
}

//z軸を任意のベクトル(origin)に向ける行列を計算しその逆行列を計算する
//得た逆行列とz軸を特定の方向(lookat)に向ける行列を計算し、乗算すると結果を得られる
XMMATRIX PmdBone::LookAtMatrix(const XMVECTOR& origin, const XMVECTOR& lookat, XMFLOAT3& up, XMFLOAT3& right)
{
	return XMMatrixTranspose(LookAtMatrix(origin, up, right))
			* LookAtMatrix(lookat, up, right);
}
