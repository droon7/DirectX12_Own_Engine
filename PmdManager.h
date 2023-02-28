#ifndef PMDMANAGER_H
#define PMDMANAGER_H

#include"pch.h"

//PMDヘッダー構造体
struct PMDHeader
{
	float version;
	char model_name[20];
	char comment[256];
};

//PMDマテリアル構造体、PMDマテリアルデータの読み込みのために使う
//パディングがあるため#pragma pack(1)でアライメントを詰める
#pragma pack(1)
struct PMDMaterialForLoad
{
	DirectX::XMFLOAT3 diffuse;   //ディフューズの色
	float alpha;				 //ディフューズα
	float specularity;			 //スペキュラの強さ
	DirectX::XMFLOAT3 specular;  //スペキュラの色
	DirectX::XMFLOAT3 ambient;   //アンビエント色
	unsigned char toonIdx;       //トゥーン番号
	unsigned char edgeFlag;      //マテリアル毎の輪郭線フラグ
	// 2 byte padding
	unsigned int indicesNum;     //このマテリアルが割り当てられるインデックス数

	char texFilePath[20];        //テクスチャファイルパス＋α
};
#pragma pack()

//シェーダー用マテリアルデータ
struct MaterialForHlsl
{
	DirectX::XMFLOAT3 diffuse;
	float alpha;
	DirectX::XMFLOAT3 specular;
	float specularity;
	DirectX::XMFLOAT3 ambient;
};

//その他マテリアルデータ
struct AdditionalMaterial
{
	std::string texPath;
	int toonIdx;
	bool edgeflag;
};

//マテリアルデータをまとめる
struct Material
{
	unsigned int indicesNum;
	MaterialForHlsl material;
	AdditionalMaterial additional;
};


//PMDモデルのデータをまとめたクラス
class PmdData
{
public:
	PmdData() {};
	
	//頂点数、インデックス数、マテリアル数格納
	unsigned int vertNum = 0;
	unsigned int indicesNum = 0;
	unsigned int materialNum = 0;

	//頂点、インデックス、マテリアルのデータ
	PMDHeader pmdHeader = {};
	std::vector<unsigned char> vertices;
	std::vector<unsigned short> indices;
	std::vector<Material> materials;
	static constexpr size_t pmdvertex_size = 38;


	//ファイルパスからPMDモデルデータをロードする。
	void loadPmdData(std::string srcModelPath);
};




#endif