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
	unsigned int frameNo;
	DirectX::XMVECTOR quaternion;

	Motion(unsigned int frameno, DirectX::XMVECTOR& q)
		: frameNo(frameno), quaternion(q)
	{
	}
};


class VMDData
{
public:
	unsigned int motionDataNum;
	unsigned int duration;
	std::vector<VMDMotionData> vmdMotionDatas;

	//���`�������[�V�������
	std::unordered_map<std::string, std::vector<Motion>> motionDatas;
	
	VMDData() {}
	VMDData(std::string strVMDPath);

	void LoadVMDData(std::string strVMDPath);

	void SetMotionDatas();
};

#endif