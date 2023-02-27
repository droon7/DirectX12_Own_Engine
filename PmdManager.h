#ifndef PMDMANAGER_H
#define PMDMANAGER_H

#include<vector>
#include<string>

#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<d3dcompiler.h>
#include<DirectXTex.h>
#include<d3dx12.h>


#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTex.lib")

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
struct PMDMaterial
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

struct PMDData
{
	unsigned int vertNum;
	unsigned int indicesNum;
	unsigned int materialNum;

	PMDHeader pmdHeader;
	std::vector<unsigned char> vertices;
	std::vector<unsigned short> indices;
	std::vector<Material> materials;
};

class PmdLoader
{
private:
	PMDData pmdData;


public:
	PmdLoader() {};



	static constexpr size_t pmdvertex_size = 38;
	PMDData getPMDData();
	PMDData loadPmdData(std::string srcModelPath);

};




#endif