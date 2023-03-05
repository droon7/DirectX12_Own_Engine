#ifndef PMDBONE_H
#define PMDBONE_H
#include"PmdManager.h"
#include"VMDData.h"
//�{�[�����A�s����Ǘ��A�ݒ�

struct BoneNode
{
	int boneIdx;					//��ӂȃ{�[��ID
	DirectX::XMFLOAT3 startPos;		//�{�[���n�_
	DirectX::XMFLOAT3 endPos;		//�{�[����[�_
	std::vector<BoneNode*> children;//�q�m�[�h
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

	//PmdActor����{�[���f�[�^�����炤
	//�e�q�֌W�܂߂��{�[���m�[�h�e�[�u�������
	void CreateBoneNodeTable(std::vector<PmdBoneData> pmdBoneDatas);
	
	void InitBoneMatrices(std::vector<PmdBoneData> pmdBoneDatas);

	void SetBoneMatrices(VMDData vmdData, unsigned int frameNo);

	//�e�m�[�h����q�m�[�h�܂ōċA�I�ɕϊ��s���������B
	void RecursiveMatrixMultiply(BoneNode* node, const DirectX::XMMATRIX& mat);
};


#endif
