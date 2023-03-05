#ifndef VMDDATA_H
#define VMDDATA_H
//モーションデータを管理する

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
	std::vector<VMDMotionData> vmdMotionDatas;

	//整形したモーション情報
	std::unordered_map<std::string, std::vector<Motion>> motionDatas;
	

	VMDData();

	void LoadVMDData(std::string strVMDPath);

	void SetMotionDatas();
};

#endif