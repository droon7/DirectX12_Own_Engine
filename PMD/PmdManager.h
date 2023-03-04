#ifndef PMDMANAGER_H
#define PMDMANAGER_H
//PMDデータの管理をする
//現在PMDデータのロードするクラスとpmd構造体について記述


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
struct MaterialData
{
	unsigned int indicesNum;
	MaterialForHlsl material;
	AdditionalMaterial additional;
};

//PMDBoneデータ構造体
//パディングをpack(1)で詰める
#pragma pack(1)
struct PmdBoneData
{
	char boneName[20];		//ボーン名
	unsigned short parentNo;//親ナンバー
	unsigned short nextNo;  //先端のボーンナンバー
	unsigned char  Type;	//ボーン種別
	unsigned short ikBoneNo;//IKボーンナンバー
	DirectX::XMFLOAT3 pos;			//ボーン基準座標
};
#pragma pack()

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
	std::vector<MaterialData> materialDatas;
	static constexpr size_t pmdvertex_size = 38;

	//ボーンデータ
	unsigned short boneNum = 0;
	std::vector<PmdBoneData> pmdBoneDatas;

	//ファイルパスからPMDモデルデータをロードする。
	void loadPmdData(std::string srcModelPath);
};




#endif