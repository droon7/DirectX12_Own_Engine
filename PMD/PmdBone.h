#ifndef PMDBONE_H
#define PMDBONE_H
#include"PmdManager.h"

struct BoneNode
{
	int boneIdx;					//一意なボーンID
	DirectX::XMFLOAT3 startPos;		//ボーン始点
	DirectX::XMFLOAT3 endPos;		//ボーン先端点
	std::vector<BoneNode*> children;//子ノード
};

class PmdBone
{
private:
	std::vector<std::string> boneNames;
	std::map<std::string, BoneNode> boneNodeTable;

public:
	PmdBone();
	PmdBone(std::vector<PmdBoneData> pmdBoneDatas);
	std::vector<DirectX::XMMATRIX> boneMatrices;

	//PmdActorからボーンデータをもらう
	//親子関係含めたボーンノードテーブルを作る
	void CreateBoneNodeTable(std::vector<PmdBoneData> pmdBoneDatas);
	
	void InitBoneMatrices(std::vector<PmdBoneData> pmdBoneDatas);

	void SetBoneMatrices();

	//親ノードから子ノードまで再帰的に変換行列をかける。
	void RecursiveMatrixMultiply(BoneNode* node, const DirectX::XMMATRIX& mat);
};


#endif
