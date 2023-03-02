#ifndef APPLICATIONL_H
#define APPLICATIONL_H

#include"pch.h"
#include"PmdManager.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTex.lib")


using Microsoft::WRL::ComPtr;

//シェーダーに渡すための基本的な行列データ
struct SceneMatrix
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
	DirectX::XMFLOAT3 eye;
};


class Application
{
private:

	//シングルトンクラスにするためコンストラクタをprivate
	Application(UINT window_width, UINT window_height);

	//~Application() {
	//	debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
	//	debugDevice->Release();
	//};

	//コピーコンストラクタと代入演算子を禁止
	Application(const Application&) = delete;
	void operator=(const Application&) = delete;

public:
	int frame = 0;

	//Directxパイプラインオブジェクトの宣言
	D3D12_RECT scissorrect = {};
	D3D12_VIEWPORT viewport = {};
	ComPtr<IDXGIFactory6> _dxgiFactory ;
	ComPtr<IDXGISwapChain4> _swapchain ;
	ComPtr<ID3D12CommandAllocator> _cmdAllocator ;
	ComPtr<ID3D12GraphicsCommandList> _cmdList = nullptr;
	ComPtr<ID3D12CommandQueue> _cmdQueue = nullptr;
	ComPtr<ID3D12DescriptorHeap> rtvHeaps = nullptr;
	ComPtr<ID3D12RootSignature> rootsignature = nullptr;
	ComPtr<ID3D12PipelineState> _pipelinestate = nullptr;
	std::vector<ID3D12Resource*> _backBuffers;


	//アセットの宣言
	ComPtr<ID3D12DescriptorHeap> matrixCsvHeaps = nullptr;

	//消す予定
	//ComPtr<ID3D12Resource> vertBuff = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	//ComPtr<ID3D12Resource> idxBuff = nullptr;
	D3D12_INDEX_BUFFER_VIEW ibView = {};

	ComPtr<ID3D12Resource> constBuff = nullptr;
	ComPtr<ID3D12Resource> depthBuffer = nullptr;
	ComPtr<ID3D12DescriptorHeap> dsvHeaps = nullptr;

	//テクスチャアセット
	D3D12_TEXTURE_COPY_LOCATION src = {};
	D3D12_TEXTURE_COPY_LOCATION dst = {};
	std::vector<ComPtr<ID3D12Resource>> textureResource;
	std::vector<ComPtr<ID3D12Resource>> sphResources;
	std::vector<ComPtr<ID3D12Resource>> spaResources;
	std::vector<ComPtr<ID3D12Resource>> toonResources;
	DirectX::TexMetadata metadata;
	DirectX::ScratchImage scratchImg;
	std::map<std::string, ComPtr<ID3D12Resource>> _resourceTable;
	ComPtr<ID3D12Resource> whiteTex;
	ComPtr<ID3D12Resource> blackTex;
	ComPtr<ID3D12Resource> gradTex;

	//行列アセット
	//DirectX::XMMATRIX matrix;
	DirectX::XMMATRIX worldMat;
	DirectX::XMMATRIX viewMat;
	DirectX::XMMATRIX projMat;
	SceneMatrix* mapMatrix = nullptr;
	float angle = 0.0f;

	//マテリアルデータ
	ComPtr<ID3D12Resource> materialBuff = nullptr;
	char* mapMaterial = nullptr;
	ComPtr<ID3D12DescriptorHeap> materialDescHeap = nullptr;

	//シェーダーオブジェクトの宣言
	ID3DBlob* _vsBlob = nullptr;
	ID3DBlob* _psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	//同期オブジェクトの宣言
	ComPtr<ID3D12Fence> _fence = nullptr;
	UINT64 _fenceVal = 0;
	HANDLE _fenceevent;

	//デバッグオブジェクトの宣言
	ID3D12DebugDevice* debugDevice;


//WICテクスチャのロード
	ComPtr<ID3D12Resource> LoadTextureFromFile(std::string& texPath);

//白テクスチャの作成
	ComPtr<ID3D12Resource> CreateWhiteTexture();
	ComPtr<ID3D12Resource> CreateBlackTexture();
	ComPtr<ID3D12Resource> CreateGradationTexture();

// PMDローダー
	PmdData pmdData;



	ComPtr<ID3D12Device> _dev;

	static Application* Instance(UINT width, UINT height);
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

//ファイル名から拡張子を得るメソッド
std::string GetExtension(const std::string& path);

//テクスチャのパスをセパレーターで分離するメソッド
std::pair<std::string, std::string> SplitFileName(const std::string& path, const char splitter);

#endif