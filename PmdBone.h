#ifndef PMDBONE_H
#define PMDBONE_H
#include"PmdManager.h"

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
	std::vector<DirectX::XMMATRIX> boneMatrices;
	std::vector<std::string> boneNames;
	std::map<std::string, BoneNode> boneNodeTable;

public:

	//PmdActor����{�[���f�[�^�����炤
	//�e�q�֌W�܂߂��{�[���m�[�h�e�[�u�������
	void CreateBoneNodeTable(std::vector<PmdBoneData> pmdBoneDatas);
	
	void InitBoneMatrices(std::vector<PmdBoneData> pmdBoneDatas);

};


#endif
