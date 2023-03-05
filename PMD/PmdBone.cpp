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

void PmdBone::SetBoneMatrices()
{
	auto node = boneNodeTable["左腕"];
	auto pos = node.startPos;

	auto matrix = DirectX::XMMatrixTranslation(-pos.x, -pos.y, -pos.z)
		* DirectX::XMMatrixRotationZ(DirectX::XM_PIDIV2)
		* DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);

	RecursiveMatrixMultiply(&node, matrix);
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
