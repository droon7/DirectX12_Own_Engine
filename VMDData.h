#ifndef VMDDATA_H
#define VMDDATA_H
//���[�V�����f�[�^���Ǘ�����
//VMD�̃f�[�^�����[�h����̂����݂̖���

//VMD���f�[�^
struct VMDMotionData
{
	char boneName[15];
	unsigned int frameNo;
	DirectX::XMFLOAT3 location;
	DirectX::XMFLOAT4 quaternion;
	unsigned char bezier[64];
};

//���\��f�[�^ 23�o�C�g�Ȃ̂�pragma�f�B���N�e�B�u�Ńp�b�L���O
#pragma pack(1)
struct VMDMorph
{
	char name[15];
	uint32_t frameNo;
	float weight;
};
#pragma pack()

//�J�����f�[�^ 61�o�C�g
#pragma pack(1)
struct VMDCamera
{
	uint32_t frameNo;
	float distance;
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 eulerAngle;
	uint8_t Interpolation[24]; //���
	uint32_t fov;
	uint8_t persflag;		   //�p�[�X�t���O
};
#pragma pack()

//���C�g�f�[�^
struct VMDLight
{
	uint32_t frameNo;
	DirectX::XMFLOAT3 rgb; //���C�g�J���[
	DirectX::XMFLOAT3 vector; //�����x�N�g��
};

//�Z���t�V���h�E�f�[�^
#pragma pack(1)
struct VMDSelfShadow
{
	uint32_t frameNo;
	uint8_t mode;		//�e���[�h�i0:�e�����A1:���[�h1�A2:���[�h2�j
	float distance;
};
#pragma pack()

//IK�I��/�I�t�f�[�^
struct VMDIKEnable
{
	uint32_t frameNo; 
	std::unordered_map<std::string, bool> ikEnableTable;
};


//���[�V�����f�[�^���`
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
private:
	unsigned int motionDataNum;					//���[�V�����̐�
	std::vector<VMDMotionData> vmdMotionDatas;
	std::vector<VMDMorph> vmdMorphs;
	std::vector<VMDCamera> vmdCameras;
	std::vector<VMDLight> vmdLights;
	std::vector<VMDSelfShadow> vmdSelfShadows;

	void LoadVMDData(std::string strVMDPath);
	void SetMotionDatas();
public:

	unsigned int duration;						//���[�V�����̒���
	//���[�V�����e�[�u���A�{�[�����ɕ����̃��[�V�����\���̂��Ή�����B
	std::unordered_map<std::string, std::vector<Motion>> motionDatas;
	std::vector<VMDIKEnable> vmdIkEnableDatas;
	
	VMDData() :motionDataNum(0), duration(0){}
	VMDData(std::string strVMDPath);


};

#endif