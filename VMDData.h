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
	DirectX::XMFLOAT2 controlPoint1, controlPoint2;

	Motion(unsigned int frameno, DirectX::XMVECTOR& q, const DirectX::XMFLOAT2& cp1, const DirectX::XMFLOAT2& cp2)
		: frameNo(frameno), quaternion(q), controlPoint1(cp1), controlPoint2(cp2)
	{
	}
};


class VMDData
{
public:
	unsigned int motionDataNum;
	unsigned int duration;
	std::vector<VMDMotionData> vmdMotionDatas;

	//モーションテーブル、ボーン名に複数のモーション構造体が対応する。
	std::unordered_map<std::string, std::vector<Motion>> motionDatas;
	
	VMDData() :motionDataNum(0), duration(0){}
	VMDData(std::string strVMDPath);

	void LoadVMDData(std::string strVMDPath);

	void SetMotionDatas();
};

#endif