#ifndef VMDDATA_H
#define VMDDATA_H
//モーションデータを管理する
//VMDのデータをロードするのが現在の役割

//VMD生データ
struct VMDMotionData
{
	char boneName[15];
	unsigned int frameNo;
	DirectX::XMFLOAT3 location;
	DirectX::XMFLOAT4 quaternion;
	unsigned char bezier[64];
};

//生表情データ 23バイトなのでpragmaディレクティブでパッキング
#pragma pack(1)
struct VMDMorph
{
	char name[15];
	uint32_t frameNo;
	float weight;
};
#pragma pack()

//カメラデータ 61バイト
#pragma pack(1)
struct VMDCamera
{
	uint32_t frameNo;
	float distance;
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 eulerAngle;
	uint8_t Interpolation[24]; //補間
	uint32_t fov;
	uint8_t persflag;		   //パースフラグ
};
#pragma pack()

//ライトデータ
struct VMDLight
{
	uint32_t frameNo;
	DirectX::XMFLOAT3 rgb; //ライトカラー
	DirectX::XMFLOAT3 vector; //光線ベクトル
};

//セルフシャドウデータ
#pragma pack(1)
struct VMDSelfShadow
{
	uint32_t frameNo;
	uint8_t mode;		//影モード（0:影無し、1:モード1、2:モード2）
	float distance;
};
#pragma pack()

//IKオン/オフデータ
struct VMDIKEnable
{
	uint32_t frameNo; 
	std::unordered_map<std::string, bool> ikEnableTable;
};


//モーションデータ整形
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
private:
	unsigned int motionDataNum;					//モーションの数
	std::vector<VMDMotionData> vmdMotionDatas;
	std::vector<VMDMorph> vmdMorphs;
	std::vector<VMDCamera> vmdCameras;
	std::vector<VMDLight> vmdLights;
	std::vector<VMDSelfShadow> vmdSelfShadows;

	void LoadVMDData(std::string strVMDPath);
	void SetMotionDatas();
public:

	unsigned int duration;						//モーションの長さ
	//モーションテーブル、ボーン名に複数のモーション構造体が対応する。
	std::unordered_map<std::string, std::vector<Motion>> motionDatas;
	std::vector<VMDIKEnable> vmdIkEnableDatas;
	
	VMDData() :motionDataNum(0), duration(0){}
	VMDData(std::string strVMDPath);


};

#endif