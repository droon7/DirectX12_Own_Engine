#ifndef PMDBONE_H
#define PMDBONE_H
#include"PmdData.h"
#include"VMDData.h"
//�{�[�����A�s����Ǘ��A�ݒ�

namespace
{
	//�{�[����ʗ񋓑�
	enum class BoneType
	{
		Rotation,
		RotAndMove,
		IK,
		Undefined,
		IKChild,
		RotationChild,
		IkDestination,
		Invisible
	};
}

struct BoneNode
{
	int boneIdx;					//��ӂȃ{�[��ID
	uint32_t boneType;				//�{�[�����
	uint32_t ikParentBone;			//IK�e�{�[��
	DirectX::XMFLOAT3 startPos;		//�{�[���n�_
	//DirectX::XMFLOAT3 endPos;		//�{�[����[�_
	std::vector<BoneNode*> children;//�q�m�[�h
};

class PmdBone
{
private:
	std::vector<std::string> boneNames;			  //�{�[��ID�ɑΉ������{�[�������i�[�����f�[�^
	std::vector<BoneNode*> boneNodeAddressArray;  //�{�[��ID�ɑΉ������{�[���m�[�h���i�[�����f�[�^
	std::map<std::string, BoneNode> boneNodeTable;//�{�[�������L�[�ɂ��ă{�[���m�[�h���i�[�����A�z�z��e�[�u��

	VMDData vmdData;	//�����o��VMDData�N���X������
	std::vector<PMDIK> motionIKs;

public:
	PmdBone();
	//�{�[���e�[�u���A��]�s��AIK���A���[�V������������������
	PmdBone(std::vector<PmdBoneData> pmdBoneDatas,std::vector<PMDIK> pmdIks, std::string motionPath);
	std::vector<DirectX::XMMATRIX> boneMatrices;	//���ۂ̉�]�s����i�[�����f�[�^

	//PmdActor����{�[���f�[�^�����炤
	//�e�q�֌W�܂߂��{�[���m�[�h�e�[�u�������
	void CreateBoneNodeTable(std::vector<PmdBoneData> pmdBoneDatas);
	
	//IK�������̃N���X�Ɋi�[����B
	void LoadPmdIks(std::vector<PMDIK> pmdIk);

	//�s�񏉊���
	void InitBoneMatrices(std::vector<PmdBoneData> pmdBoneDatas);

	//�t���[�������ăL�[�t���[���𔭌��A�O�L�[�t���[���ƕ�Ԃ���]�s������肷��B���t���[���Ăяo��
	void SetBoneMatrices(unsigned int frameNo);

	//�e�m�[�h����q�m�[�h�܂ōċA�I�ɕϊ��s���������B
	void RecursiveMatrixMultiply(BoneNode* node, const DirectX::XMMATRIX& mat);

	//�x�W�F�Ȑ����ȒP�ȋߎ��v�Z�ŋ��߂�
	float GetYFromXOnBezier(float x, const DirectX::XMFLOAT2& controlPoint1, const DirectX::XMFLOAT2& controlPoint2, uint8_t max_steps);

	//���[�v�A�j���[�V�����̂��߃��[�V�����̒����𓾂�
	inline unsigned int GetMotionDataDuration()
	{
		return vmdData.duration;
	}

	//IK�\���o�[
	void IKSolve();

	//CCD-IK�ɂ��{�[������������
	void SolveCCDIK(const PMDIK& ik);

	//�]���藝IK�ɂ��{�[������������
	void SolveCosineIK(const PMDIK& ik);

	//LookAt�s��ɂ��{�[������������
	void SolveLookAt(const PMDIK& ik);

	//Z�������̕����Ɍ�����s���Ԃ��Bup��right�͕⏕�x�N�g���Ƃ��Ă̏�x�N�g���ƉE�x�N�g��
	DirectX::XMMATRIX LookAtMatrix(const DirectX::XMVECTOR& lookat, DirectX::XMFLOAT3& up, DirectX::XMFLOAT3& right);

	//����̃x�N�g�������̕����Ɍ����邽�߂̍s���Ԃ�
	DirectX::XMMATRIX LookAtMatrix(const DirectX::XMVECTOR& origin, const DirectX::XMVECTOR& lookat, DirectX::XMFLOAT3& up, DirectX::XMFLOAT3& right);
};


#endif
