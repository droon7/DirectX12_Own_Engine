#ifndef PMDBONE_H
#define PMDBONE_H
#include"PmdData.h"
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
	
	//�s�񏉊���
	void InitBoneMatrices(std::vector<PmdBoneData> pmdBoneDatas);

	//�t���[�������ăL�[�t���[���𔭌��A�O�L�[�t���[���ƕ�Ԃ���]�s������肷��B���t���[���Ăяo��
	void SetBoneMatrices(VMDData vmdData, unsigned int frameNo);

	//�e�m�[�h����q�m�[�h�܂ōċA�I�ɕϊ��s���������B
	void RecursiveMatrixMultiply(BoneNode* node, const DirectX::XMMATRIX& mat);

	//�x�W�F�Ȑ����ȒP�ȋߎ��v�Z�ŋ��߂�
	float GetYFromXOnBezier(float x, const DirectX::XMFLOAT2& controlPoint1, const DirectX::XMFLOAT2& controlPoint2, uint8_t max_steps);

};


#endif
