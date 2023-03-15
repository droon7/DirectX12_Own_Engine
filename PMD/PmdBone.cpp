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
		if (boneNames[idx].find("ひざ") != std::string::npos)
		{
			kneeIdxes.emplace_back(idx);
		}

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

//フォワードキネマティクスの実装部分
//フレームを見てキーフレームを発見、前キーフレームと補間し回転行列+平行移動行列を決定する。
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
		auto isMoreThanPredicate = [frameNo](const Motion& motion) {
			return motion.frameNo <= frameNo;
		};

		//近い値のイテレーターを得る、昇順に並んでいるデータを探索するのでリバースイテレーターを使う
		auto rit = std::find_if(
			motions.rbegin(), motions.rend(), isMoreThanPredicate
		);

		//モーションデータがないならcontinue
		if (rit == motions.rend())
		{
			continue;
		}

		//前キーフレームと次キーフレームで補間を行う
		//回転データ(quaternion)とロケーションデータ(offset)で行列を計算する
		XMMATRIX rotation;
		XMVECTOR offset = XMLoadFloat3(&rit->offset);
		auto it = rit.base();

		if (it == motions.end())
		{
			rotation = XMMatrixRotationQuaternion(rit->quaternion);
		}
		else 
		{
			//コントロールポイントからベジェ曲線を計算し、補間係数を計算
			auto t = static_cast<float>(frameNo - rit->frameNo)
				/ static_cast<float>(it->frameNo - rit->frameNo);
			t = GetYFromXOnBezier(t, it->controlPoint1, it->controlPoint2, 12);

			//線形補間を計算
			rotation = XMMatrixRotationQuaternion(XMQuaternionSlerp(rit->quaternion, it->quaternion, t));
			offset = XMVectorLerp(offset, XMLoadFloat3(&it->offset), t);
		}

		//得た行列を格納。センターから再帰で伝搬させる。
		//回転は原点で行う
		auto& pos = node.startPos;
		auto matrix = XMMatrixTranslation(-pos.x, -pos.y, -pos.z)
			* rotation
			* XMMatrixTranslation(pos.x, pos.y, pos.z);
		boneMatrices[node.boneIdx] = matrix * XMMatrixTranslationFromVector(offset);
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

void PmdBone::IKSolve(uint32_t frameNo)
{
	for (auto& ik : motionIKs)
	{
		//IKスイッチチェック
		if (isOnIkInFrame(frameNo, ik) == false)
		{
			continue;
		}

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
	//最終ターゲットの位置を格納
	auto targetBoneNode = boneNodeAddressArray[ik.boneIdx];
	auto targetOriginPos = XMLoadFloat3(&targetBoneNode->startPos);

	auto parentMat = boneMatrices[boneNodeAddressArray[ik.boneIdx]->ikParentBone];
	//一時的にIKの親の回転行列を無視するため逆行列を計算し適用
	XMVECTOR determinant;
	auto inverseParentMat = XMMatrixInverse(&determinant, parentMat);
	auto targetNextPos = XMVector3Transform(targetOriginPos, boneMatrices[ik.boneIdx] * inverseParentMat);

	//末端ノード位置
	auto endPos = XMLoadFloat3(&boneNodeAddressArray[ik.targetIdx]->startPos);

	//CCD-IK用の各ノードの位置、回転行列を格納
	std::vector<XMVECTOR> positions;
	std::vector<XMMATRIX> matrices;
	matrices.resize(ik.nodeIdx.size());
	for (auto& nodeId : ik.nodeIdx)
	{
		positions.emplace_back(XMLoadFloat3(&boneNodeAddressArray[nodeId]->startPos));
	}
	fill(matrices.begin(), matrices.end(), XMMatrixIdentity());

	//許容誤差
	float epsilon = 0.005f;

	//CCD-IK本体
	for (int c = 0; c < ik.iterations; ++c)
	{
		if (XMVector3Length(XMVectorSubtract(endPos, targetNextPos)).m128_f32[0] <= epsilon)
		{
			break;
		}

		//角度制限を考慮し、ノードをさかのぼりボーンを曲げていく
		for (int nodeId = 0; nodeId < positions.size(); ++nodeId)
		{
			const auto& pos = positions[nodeId];

			//現在のノードと末端ノード、目的位置へのベクトルをとる
			auto vectorToEnd = XMVectorSubtract(endPos, pos);
			auto vectorToTarget = XMVectorSubtract(targetNextPos, pos);
			vectorToEnd = XMVector3Normalize(vectorToEnd);
			vectorToTarget = XMVector3Normalize(vectorToTarget);

			//上のベクトルが同じならできることは無いため次のノードヘ
			if (XMVector3Length(XMVectorSubtract(vectorToEnd, vectorToTarget)).m128_f32[0] <= epsilon)
			{
				continue;
			}

			//座標軸とベクトル間角度を得る
			auto cross = XMVector3Normalize(XMVector3Cross(vectorToEnd, vectorToTarget));
			float angle = XMVector2AngleBetweenVectors(vectorToEnd, vectorToTarget).m128_f32[0];

			//回転角制限を考慮
			angle = min(angle, ik.limit);
			XMMATRIX rotation = XMMatrixRotationAxis(cross, angle);

			//pos中心に回転?
			auto matrix = XMMatrixTranslationFromVector(-pos)
				* rotation
				* XMMatrixTranslationFromVector(pos);

			//行列を掛け続け最終的な回転行列を作る
			matrices[nodeId] *= matrix;

			//自分より先端側にある点を全て今回計算した行列で回転する。
			for (auto id = nodeId; id >= 0; --id)
			{
				positions[id] = XMVector3Transform(positions[id], matrix);
			}
			endPos = XMVector3Transform(endPos, matrix);


			//許容誤差以内なら終了
			if (XMVector3Length(XMVectorSubtract(endPos, targetNextPos)).m128_f32[0] <= epsilon)
			{
				break;
			}
		}
	}

	//CCD-IK終了後にボーンクラスの回転行列に反映
	int id = 0;
	for (auto& nodeId : ik.nodeIdx)
	{
		boneMatrices[nodeId] = matrices[id];
		id++;
	}

	//無視した親の行列を戻し再計算
	auto rootNode = boneNodeAddressArray[ik.nodeIdx.back()];
	RecursiveMatrixMultiply(rootNode, parentMat);
}

//余弦定理によるIK
void PmdBone::SolveCosineIK(const PMDIK& ik)
{
	//各ノードの位置、及びボーン長さを格納
	std::vector<XMVECTOR> positions;
	std::array<float, 2> edgeLens;

	//最終ターゲット位置を格納
	auto& targetNode = boneNodeAddressArray[ik.boneIdx];
	auto targetPos = XMVector3Transform(XMLoadFloat3(&targetNode->startPos), boneMatrices[ik.boneIdx]);

	//末端ボーンの座標格納
	auto endNode = boneNodeAddressArray[ik.targetIdx];
	positions.emplace_back(XMLoadFloat3(&endNode->startPos));

	//中間及びルートボーンの座標格納
	for (auto& chainBoneIdx : ik.nodeIdx)
	{
		auto boneNode = boneNodeAddressArray[chainBoneIdx];
		positions.emplace_back(XMLoadFloat3(&boneNode->startPos));
	}

	//わかりやすさのため逆順に
	reverse(positions.begin(), positions.end());

	edgeLens[0] = XMVector3Length(XMVectorSubtract(positions[1], positions[0])).m128_f32[0];
	edgeLens[1] = XMVector3Length(XMVectorSubtract(positions[2], positions[1])).m128_f32[0];

	//ルートボーンをFKで得た現在の座標に変換
	positions[0] = XMVector3Transform(positions[0], boneMatrices[ik.nodeIdx[1]]);
	//先端ボーンをFKで得た現在の座標に変換
	positions[2] = XMVector3Transform(positions[2], boneMatrices[ik.boneIdx]);

	//ルートから先端へのベクトルの作成
	auto linearVec = XMVectorSubtract(positions[2], positions[0]);

	//余弦定理の計算の準備
	float A = XMVector3Length(linearVec).m128_f32[0];
	float B = edgeLens[0];
	float C = edgeLens[1];

	linearVec = XMVector3Normalize(linearVec);
	
	//ルートノードにおける角の計算
	float theta1 = acosf((A * A + B * B - C * C) / (2 * A * B));
	//真ん中ノードにおける角の計算
	float theta2 = acosf((B * B + C * C - A * A) / (2 * B * C));

	//回転軸の生成、真ん中のノードが膝ノードならx軸を回転軸とする
	XMVECTOR axis;
	if (std::find(kneeIdxes.begin(), kneeIdxes.end(), ik.nodeIdx[0]) == kneeIdxes.end())
	{
		//3つのノードがなす平面の法線ベクトルが回転軸
		//XXX : 3つのノードが直線上に並んでいるとクラッシュ
		auto vm = XMVector3Normalize(XMVectorSubtract(positions[2], positions[0]));
		auto vt = XMVector3Normalize(XMVectorSubtract(targetPos, positions[0]));
		axis = XMVector3Cross(vm, vt);
	}
	else
	{
		auto right = XMFLOAT3(1, 0, 0);
		axis = XMLoadFloat3(&right);
	}

	//原点に戻してから回転させる
	auto mat1 = XMMatrixTranslationFromVector(-positions[0]);
	mat1 *= XMMatrixRotationAxis(axis, theta1);
	mat1 *= XMMatrixTranslationFromVector(positions[0]);

	//真ん中のノードは回転の正負が逆となるので-(pi-theta2)となる
	auto mat2 = XMMatrixTranslationFromVector(-positions[1]);
	mat2 *= XMMatrixRotationAxis(axis, -(XM_PI-theta2));
	mat2 *= XMMatrixTranslationFromVector(positions[1]);

	boneMatrices[ik.nodeIdx[1]] *= mat1;
	//ルートノードの結果にさらに回転行列を掛ける
	boneMatrices[ik.nodeIdx[0]] = mat2 * boneMatrices[ik.nodeIdx[1]];
	//末端ノードにも回転行列を伝搬
	boneMatrices[ik.targetIdx] = boneMatrices[ik.nodeIdx[0]];
}

//IK適用前のルートからターゲットへのベクトルとIK適用後のルートからターゲットへのベクトルを得る
//それらでLookAtMatrixで適用前ベクトルから適用後ベクトルへの回転行列を得て格納する
void PmdBone::SolveLookAt(const PMDIK& ik)
{
	//間点は無いため最初のノードがルートノードとなる
	auto rootNode = boneNodeAddressArray[ik.nodeIdx[0]];
	auto targetNode = boneNodeAddressArray[ik.boneIdx];
	
	auto rootPos1 = XMLoadFloat3(&rootNode->startPos);
	auto targetPos1 = XMLoadFloat3(&targetNode->startPos);

	auto rootPos2 = XMVector3Transform(rootPos1, boneMatrices[ik.nodeIdx[0]]);
	auto targetPos2 = XMVector3Transform(targetPos1, boneMatrices[ik.boneIdx]);

	auto originVec = XMVectorSubtract(targetPos1, rootPos1);
	auto targetVec = XMVectorSubtract(targetPos2, rootPos2);

	originVec = XMVector3Normalize(originVec);
	targetVec = XMVector3Normalize(targetVec);
	XMFLOAT3 up = XMFLOAT3(0, 1, 0);
	XMFLOAT3 right = XMFLOAT3(1, 0, 0);

	boneMatrices[ik.nodeIdx[0]] = LookAtMatrix(originVec, targetVec, up, right);

}

//z軸がlookatに向く回転行列を作る
//lookatと補助ベクトルで二回外積を行い、座標系を作ると正規化されたx,y,z成分が得られる。これを行列の各列に格納すると回転後座標が得られる。
XMMATRIX PmdBone::LookAtMatrix(const XMVECTOR& lookat, XMFLOAT3& up, XMFLOAT3& right)
{
	//向かせたい方向
	XMVECTOR vz = XMVector3Normalize(lookat);

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

//z軸を任意のベクトル(origin)に向ける行列を計算しその逆行列を計算する。つまりoriginをz軸に向かせる回転行列を作る(1)
//(1)とz軸をlookatに向かせる回転行列を掛け、originをlookatに向かせる回転行列を作る。
XMMATRIX PmdBone::LookAtMatrix(const XMVECTOR& origin, const XMVECTOR& lookat, XMFLOAT3& up, XMFLOAT3& right)
{
	return XMMatrixTranspose(LookAtMatrix(origin, up, right))
			* LookAtMatrix(lookat, up, right);
}


// フレーム毎IKスイッチ確認
bool PmdBone::isOnIkInFrame(uint32_t frameNo, PMDIK& ik)
{
	auto iterator = std::find_if(vmdData.vmdIkEnableDatas.rbegin(), vmdData.vmdIkEnableDatas.rend(),
		[frameNo](const VMDIKEnable& ikEnable)
		{
			return ikEnable.frameNo <= frameNo;
		});

	//IKスイッチにあり、IKスイッチがfalseならfalseを返す
	if (iterator != vmdData.vmdIkEnableDatas.rend())
	{
		auto ikEnableIt = iterator->ikEnableTable.find(boneNames[ik.boneIdx]);

		if (ikEnableIt != iterator->ikEnableTable.end())
		{
			if (ikEnableIt->second == false)
			{
				return false;
			}
		}
	}


	return true;
}