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
	unsigned int frameNo;			//キーフレーム番号
	DirectX::XMVECTOR quaternion;   //クォータニオン
	DirectX::XMFLOAT3 offset;		//IKの初期座標からのオフセット情報
	DirectX::XMFLOAT2 controlPoint1, controlPoint2; //ベジェ曲線の中間コントロールポイント

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
	unsigned int motionDataNum;					//モーションの数
	unsigned int duration;						//モーションの長さ
	std::vector<VMDMotionData> vmdMotionDatas;

	//モーションテーブル、ボーン名に複数のモーション構造体が対応する。
	std::unordered_map<std::string, std::vector<Motion>> motionDatas;
	
	VMDData() :motionDataNum(0), duration(0){}
	VMDData(std::string strVMDPath);

	void LoadVMDData(std::string strVMDPath);

	void SetMotionDatas();
};

#endif