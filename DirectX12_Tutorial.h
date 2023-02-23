#ifndef TUTORIAL_H
#define TUTORIAL_H

#include <Windows.h>
#include <tchar.h>
#include<vector>
#include<string>
#include<wrl.h>
#ifdef _DEBUG
#include <iostream>
#endif

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


using Microsoft::WRL::ComPtr;

//PMDヘッダー構造体
struct PMDHeader
{
	float version;
	char model_name[20];
	char comment[256];
};

//シェーダーに渡すための基本的な行列データ
struct MatricesData
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX viewproj;
};

//PMDマテリアル構造体、PMDマテリアルデータの読み込みのために使う
//パディングがあるため#pragma pack(1)でアライメントを詰める
#pragma pack(1)
struct PMDMaterial
{
	DirectX::XMFLOAT3 diffuse;   //ディフューズの色
	float alpha;				 //ディフューズα
	DirectX::XMFLOAT3 specular;  //スペキュラの色
	float specularity;			 //スペキュラの強さ
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

class Dx12
{
public:
	Dx12(UINT window_width, UINT window_height);
	//~Dx12();

	void OnInit();
	void OnUpdate();
	void OnRender();
	void OnDestroy();
	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandList();
	void WaitForPreviousFrame();

	//ウィンドウサイズ
	UINT window_width;
	UINT window_height;
	//レンダーターゲットとなるバッファの数
	static const int buffer_count = 2;

private:
	int frame = 0;

	//Directxパイプラインオブジェクトの宣言
	D3D12_RECT scissorrect = {};
	D3D12_VIEWPORT viewport = {};
	ComPtr<ID3D12Device> _dev ;
	ComPtr<IDXGIFactory6> _dxgiFactory ;
	ComPtr<IDXGISwapChain4> _swapchain ;
	ComPtr<ID3D12CommandAllocator> _cmdAllocator ;
	ComPtr<ID3D12GraphicsCommandList> _cmdList = nullptr;
	ComPtr<ID3D12CommandQueue> _cmdQueue = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	ComPtr<ID3D12DescriptorHeap> rtvHeaps = nullptr;
	ComPtr<ID3D12DescriptorHeap> basicDescHeaps = nullptr;
	ComPtr<ID3D12RootSignature> rootsignature = nullptr;
	ComPtr<ID3D12PipelineState> _pipelinestate = nullptr;
	std::vector<ID3D12Resource*> _backBuffers;

	//アセットの宣言

	ComPtr<ID3D12Resource> vertBuff = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	ComPtr<ID3D12Resource> idxBuff = nullptr;
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	D3D12_TEXTURE_COPY_LOCATION src = {};
	D3D12_TEXTURE_COPY_LOCATION dst = {};
	std::vector<ComPtr<ID3D12Resource>> textureResource;
	ComPtr<ID3D12Resource> whiteTex;
	ComPtr<ID3D12Resource> constBuff = nullptr;
	ComPtr<ID3D12Resource> depthBuffer = nullptr;
	ComPtr<ID3D12DescriptorHeap> dsvHeaps = nullptr;

	DirectX::TexMetadata metadata;
	DirectX::ScratchImage scratchImg;

	//行列アセット
	DirectX::XMMATRIX matrix;
	DirectX::XMMATRIX worldMat;
	DirectX::XMMATRIX viewMat;
	DirectX::XMMATRIX projMat;
	MatricesData* mapMatrix = nullptr;
	float angle = 0.0f;

	//PMDデータの宣言
	PMDHeader pmdheader;

	static constexpr size_t pmdvertex_size = 38;
	unsigned int vertNum;
	unsigned int indicesNum;
	unsigned int materialNum;

	//マテリアルデータ
	ComPtr<ID3D12Resource> materialBuff = nullptr;
	char* mapMaterial = nullptr;
	ComPtr<ID3D12DescriptorHeap> materialDescHeap = nullptr;
	std::vector<Material> materials;

	//シェーダーオブジェクトの宣言
	ID3DBlob* _vsBlob = nullptr;
	ID3DBlob* _psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;


	//同期オブジェクトの宣言
	ComPtr<ID3D12Fence> _fence = nullptr;
	UINT64 _fenceVal = 0;
	HANDLE _fenceevent;


//WICテクスチャのロード
	ComPtr<ID3D12Resource> LoadTextureFromFile(std::string& texPath);

//白テクスチャの作成
	ComPtr<ID3D12Resource> CreateWhiteTexture();
};



//ウィンドウアプリクラス
class Win32App
{
public:
	static int WindowRun(Dx12* pdx12);
	static HWND GetHwnd() { return m_hwnd; } 

	static void DebugOutputFormatString(const char* format, ...); //ウィンドウデバッグ用関数
	LRESULT static WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam); //ウィンドウ初期化関数

private:
	 static HWND m_hwnd;
};


//端数を切り捨てるメソッド
inline size_t AlignmentedSize(size_t size, size_t alignment)
{
	return size + alignment - size % alignment;
};

//モデルパスからファイル名を取り除き、テクスチャパスと合成するメソッド
inline std::string GetTexturePathFromModelAndTexPath(const std::string& modelPath, const char* texPath)
{
	auto folderPath = modelPath.substr(0, modelPath.rfind('/')+1);
	return folderPath + texPath;
}

//文字列からワイド文字列を得るメソッド
std::wstring GetWideStringFromString(const std::string& str);


#endif