#include "pch.h"
#include "PmdBone.h"

PmdBone::PmdBone()
{
}

PmdBone::PmdBone(std::vector<PmdBoneData> pmdBoneDatas)
{
	CreateBoneNodeTable(pmdBoneDatas);
	InitBoneMatrices(pmdBoneDatas);
}

//親子関係のあるボーンノードテーブルを作る
void PmdBone::CreateBoneNodeTable(std::vector<PmdBoneData> pmdBoneDatas)
{
	boneNames.resize(pmdBoneDatas.size());

	//ボーンネームズと連想配列に全ボーンデータを入れる
	for (int idx = 0; idx < pmdBoneDatas.size(); ++idx)
	{
		auto& pmdbonedata = pmdBoneDatas[idx];
		boneNames[idx] = pmdbonedata.boneName;
		auto& node = boneNodeTable[pmdbonedata.boneName];
		node.boneIdx = idx;
		node.startPos = pmdbonedata.pos;
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

//ボーン変換行列初期化
void PmdBone::InitBoneMatrices(std::vector<PmdBoneData> pmdBoneDatas)
{
	boneMatrices.resize(pmdBoneDatas.size());

	std::fill(boneMatrices.begin(), boneMatrices.end(), DirectX::XMMatrixIdentity());


}

//フレームを見てキーフレームを発見、前キーフレームと補間し回転行列を決定する。
//TODO: この関数は分割したい。
void PmdBone::SetBoneMatrices(VMDData vmdData, unsigned int frameNo)
{
	std::fill(boneMatrices.begin(), boneMatrices.end(), DirectX::XMMatrixIdentity());

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
		DirectX::XMMATRIX rotation;
		auto it = rit.base();

		if (it == motions.end())
		{
			rotation = DirectX::XMMatrixRotationQuaternion(rit->quaternion);
		}
		else 
		{
			//ベジェ曲線補間
			auto t = static_cast<float>(frameNo - rit->frameNo)
				/ static_cast<float>(it->frameNo - rit->frameNo);
			t = GetYFromXOnBezier(t, it->controlPoint1, it->controlPoint2, 12);
			rotation = DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionSlerp(rit->quaternion, it->quaternion, t));
			//rotation = DirectX::XMMatrixRotationQuaternion(rit->quaternion);
		}

		//得た回転行列を格納。センターから再帰で伝搬させる。
		auto& pos = node.startPos;
		auto matrix = DirectX::XMMatrixTranslation(-pos.x, -pos.y, -pos.z)
			* rotation
			* DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
		boneMatrices[node.boneIdx] = matrix;
	}

	RecursiveMatrixMultiply(&boneNodeTable["センター"], DirectX::XMMatrixIdentity());
}


void PmdBone::RecursiveMatrixMultiply(BoneNode* node, const DirectX::XMMATRIX& mat)
{
	//格納されている単位行列に得た変換行列を掛け、データを保存
	boneMatrices[node->boneIdx] *= mat;

	for (auto& childNode : node->children)
	{
		RecursiveMatrixMultiply(childNode, boneMatrices[node->boneIdx]);
	}
}

float PmdBone::GetYFromXOnBezier(float x, const DirectX::XMFLOAT2& controlPoint1, const DirectX::XMFLOAT2& controlPoint2, uint8_t max_steps)
{
	if (controlPoint1.x == controlPoint1.y && controlPoint2.x == controlPoint2.y)
	{
		return x;
	}

	float t = x;
	//係数
	const float k0 = 1 + 3 * controlPoint1.x - 3 * controlPoint2.x;
	const float k1 = 3 * controlPoint2.x - 6 * controlPoint1.x;
	const float k2 = 3 * controlPoint1.x;

	//判定値
	constexpr float epsilon = 0.0005f;

	//近似値まで計算を回す
	for (int i = 0; i < max_steps; ++i)
	{
		auto ft = k0 * t * t * t + k1 * t * t + k0 * t - x;

		if (ft <= epsilon && ft >= -epsilon)
		{
			break;
		}
		t -= ft / 2;
	}

	auto r = 1 - t;
	auto ret = t * t * t + 3 * t * t * r * controlPoint2.y + 3 * t * r * r * controlPoint1.y;
	
	return ret;
}
