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
	//ボーンテーブル、回転行列、IK情報、モーション情報を初期化する
	PmdBone(std::vector<PmdBoneData> pmdBoneDatas,std::vector<PMDIK> pmdIks, std::string motionPath);
	std::vector<DirectX::XMMATRIX> boneMatrices;	//実際の回転行列を格納したデータ

	//PmdActorからボーンデータをもらう
	//親子関係含めたボーンノードテーブルを作る
	void CreateBoneNodeTable(std::vector<PmdBoneData> pmdBoneDatas);
	
	//IK情報をこのクラスに格納する。
	void LoadPmdIks(std::vector<PMDIK> pmdIk);

	//行列初期化
	void InitBoneMatrices(std::vector<PmdBoneData> pmdBoneDatas);

	//フレームを見てキーフレームを発見、前キーフレームと補間し回転行列を決定する。毎フレーム呼び出す
	void SetBoneMatrices(unsigned int frameNo);

	//親ノードから子ノードまで再帰的に変換行列をかける。
	void RecursiveMatrixMultiply(BoneNode* node, const DirectX::XMMATRIX& mat);

	//ベジェ曲線を簡単な近似計算で求める
	float GetYFromXOnBezier(float x, const DirectX::XMFLOAT2& controlPoint1, const DirectX::XMFLOAT2& controlPoint2, uint8_t max_steps);

	//ループアニメーションのためモーションの長さを得る
	inline unsigned int GetMotionDataDuration()
	{
		return vmdData.duration;
	}

	//IKソルバー
	void IKSolve();

	//CCD-IKによりボーン方向を解決
	void SolveCCDIK(const PMDIK& ik);

	//余弦定理IKによりボーン方向を解決
	void SolveCosineIK(const PMDIK& ik);

	//LookAt行列によりボーン方向を解決
	void SolveLookAt(const PMDIK& ik);

	//Z軸を特定の方向に向ける行列を返す。upとrightは補助ベクトルとしての上ベクトルと右ベクトル
	DirectX::XMMATRIX LookAtMatrix(const DirectX::XMVECTOR& lookat, DirectX::XMFLOAT3& up, DirectX::XMFLOAT3& right);

	//特定のベクトルを特定の方向に向けるための行列を返す
	DirectX::XMMATRIX LookAtMatrix(const DirectX::XMVECTOR& origin, const DirectX::XMVECTOR& lookat, DirectX::XMFLOAT3& up, DirectX::XMFLOAT3& right);
};


#endif
