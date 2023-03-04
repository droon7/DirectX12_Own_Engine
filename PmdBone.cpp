#include "pch.h"
#include "PmdBone.h"

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
