#ifndef PMDBONE_H
#define PMDBONE_H
#include"PmdData.h"
#include"VMDData.h"
//ボーン情報、行列を管理、設定

namespace
{
	//ボーン種別列挙体
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
	int boneIdx;					//一意なボーンID
	uint32_t boneType;				//ボーン種別
	uint32_t ikParentBone;			//IK親ボーン
	DirectX::XMFLOAT3 startPos;		//ボーン始点
	//DirectX::XMFLOAT3 endPos;		//ボーン先端点
	std::vector<BoneNode*> children;//子ノード
};

class PmdBone
{
private:
	std::vector<std::string> boneNames;			  //ボーンIDに対応したボーン名を格納したデータ
	std::vector<BoneNode*> boneNodeAddressArray;  //ボーンIDに対応したボーンノードを格納したデータ
	std::map<std::string, BoneNode> boneNodeTable;//ボーン名をキーにしてボーンノードを格納した連想配列テーブル

	VMDData vmdData;	//メンバにVMDDataクラスを持つ
	std::vector<PMDIK> motionIKs;

public:
	PmdBone();
	PmdBone(std::vector<PmdBoneData> pmdBoneDatas,std::vector<PMDIK> pmdIks, std::string motionPath);
	std::vector<DirectX::XMMATRIX> boneMatrices;	//実際の回転行列を格納したデータ

	//PmdActorからボーンデータをもらう
	//親子関係含めたボーンノードテーブルを作る
	void CreateBoneNodeTable(std::vector<PmdBoneData> pmdBoneDatas);
	
	void LoadPmdIks(std::vector<PMDIK> pmdIk);

	//行列初期化
	void InitBoneMatrices(std::vector<PmdBoneData> pmdBoneDatas);

	//フレームを見てキーフレームを発見、前キーフレームと補間し回転行列を決定する。毎フレーム呼び出す
	void SetBoneMatrices(unsigned int frameNo);

	//親ノードから子ノードまで再帰的に変換行列をかける。
	void RecursiveMatrixMultiply(BoneNode* node, const DirectX::XMMATRIX& mat);

	//ベジェ曲線を簡単な近似計算で求める
	float GetYFromXOnBezier(float x, const DirectX::XMFLOAT2& controlPoint1, const DirectX::XMFLOAT2& controlPoint2, uint8_t max_steps);

	inline unsigned int GetMotionDataDuration()
	{
		return vmdData.duration;
	}
};


#endif
