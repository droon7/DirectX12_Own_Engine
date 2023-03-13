#ifndef VMDDATA_H
#define VMDDATA_H
//���[�V�����f�[�^���Ǘ�����

struct VMDMotionData
{
	char boneName[15];
	unsigned int frameNo;
	DirectX::XMFLOAT3 location;
	DirectX::XMFLOAT4 quaternion;
	unsigned char bezier[64];
};

struct Motion
{
	unsigned int frameNo;			//�L�[�t���[���ԍ�
	DirectX::XMVECTOR quaternion;   //�N�H�[�^�j�I��
	DirectX::XMFLOAT3 offset;		//IK�̏������W����̃I�t�Z�b�g���
	DirectX::XMFLOAT2 controlPoint1, controlPoint2; //�x�W�F�Ȑ��̒��ԃR���g���[���|�C���g

	Motion(unsigned int frameno, 
			DirectX::XMVECTOR& q, 
			DirectX::XMFLOAT3& offs,
			const DirectX::XMFLOAT2& cp1,
			const DirectX::XMFLOAT2& cp2)
		: frameNo(frameno), quaternion(q),offset(offs), controlPoint1(cp1), controlPoint2(cp2)
	{
	}
};


class VMDData
{
public:
	unsigned int motionDataNum;					//���[�V�����̐�
	unsigned int duration;						//���[�V�����̒���
	std::vector<VMDMotionData> vmdMotionDatas;

	//���[�V�����e�[�u���A�{�[�����ɕ����̃��[�V�����\���̂��Ή�����B
	std::unordered_map<std::string, std::vector<Motion>> motionDatas;
	
	VMDData() :motionDataNum(0), duration(0){}
	VMDData(std::string strVMDPath);

	void LoadVMDData(std::string strVMDPath);

	void SetMotionDatas();
};

#endif