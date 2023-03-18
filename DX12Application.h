#ifndef APPLICATIONL_H
#define APPLICATIONL_H

#include"pch.h"
#include"PmdData.h"
#include"OtherRenderTarget.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTex.lib")
#pragma comment(lib, "Winmm.lib")


using Microsoft::WRL::ComPtr;

//シェーダーに渡すための基本的な行列データ
struct SceneMatrix
{
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
	DirectX::XMMATRIX lightCamera;  //シャドウマップ用
	DirectX::XMMATRIX shadow;		//影行列用
	DirectX::XMFLOAT3 eye;
};

class OtherRenderTarget;
class PmdRenderer;
//DirectX12の全体の設定をするクラス。
class DX12Application
{
	friend OtherRenderTarget;
	friend PmdRenderer;
private:

	//シングルトンクラスにするためコンストラクタをprivate
	DX12Application(UINT window_width, UINT window_height);

	//ReportLiveDeviceObjectsを呼ぶ
	//~Application() {
	//	debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
	//	debugDevice->Release();
	//};
	// 
	//コピーコンストラクタと代入演算子を禁止
	DX12Application(const DX12Application&) = delete;
	void operator=(const DX12Application&) = delete;

	//フレーム数
	int frame = 0;
	//ウィンドウサイズ
	UINT window_width;
	UINT window_height;
	//レンダーターゲットとなるバッファの数
	static const int buffer_count = 2;

	//Directxパイプラインオブジェクトの宣言
	D3D12_RECT scissorrect = {};
	D3D12_VIEWPORT viewport = {};
	ComPtr<IDXGIFactory6> _dxgiFactory ;
	ComPtr<ID3D12DescriptorHeap> rtvHeaps = nullptr;
	std::vector<ComPtr<ID3D12Resource>> _backBuffers;

	//アセットの宣言
	ComPtr<ID3D12DescriptorHeap> matrixCsvHeaps = nullptr;
	ComPtr<ID3D12Resource> sceneMatrixConstBuff = nullptr;

	ComPtr<ID3D12Resource> depthBuffer = nullptr;
	ComPtr<ID3D12DescriptorHeap> dsvHeaps = nullptr;



	//行列アセット
	DirectX::XMMATRIX viewMat;
	DirectX::XMMATRIX projMat;
	SceneMatrix* mapTransform = nullptr;
	float angle = 0.0f;

	//影用アセット
	static constexpr unsigned int shadowDifinition = 2048;
	DirectX::XMFLOAT3 parallelLightVec;
	DirectX::XMMATRIX lightMat;
	ComPtr<ID3D12Resource> shadowMapBuffer = nullptr; 
	ComPtr<ID3D12DescriptorHeap> depthSRVHeaps = nullptr;
	ComPtr<ID3D12PipelineState> shadowMapPls = nullptr;

	//デバッグオブジェクトの宣言
	ID3D12DebugDevice* debugDevice;

	HRESULT InitializeDXGIDevice();
	HRESULT InitializeCommands();
	HRESULT CreateSwapChain(const HWND& hwnd);
	HRESULT CreateFinalRenderTargets();
	//深度バッファビューを作る
	HRESULT CreateDepthStencilView();
	//ビュー行列、投射行列から作るシーンのビューを作る
	HRESULT CreateSceneView();

public:
	ComPtr<ID3D12Device> _dev;
	ComPtr<IDXGISwapChain4> _swapchain;
	ComPtr<ID3D12GraphicsCommandList> _cmdList = nullptr;
	ComPtr<ID3D12CommandAllocator> _cmdAllocator;
	ComPtr<ID3D12CommandQueue> _cmdQueue = nullptr;
	ComPtr<ID3D12Fence> _fence = nullptr;
	UINT64 _fenceVal = 0;
	HANDLE _fenceevent;



	void OnInit(const HWND& hwnd);
	//void OnUpdate();
	//void OnRender();
	void OnDestroy();
	void LoadPipeline();
	//void LoadAssets();
	// 
	//シングルトンパターンのためstaticの生成メソッド
	static DX12Application* Instance(UINT width, UINT height);
	//バックバッファの描画開始メソッド、レンダーターゲット、バリア、深度ビュー、ビューポートのコマンド追加
	void SetBackBufferToRTV();
	//バックバッファのリソースバリアの設定
	void EndBackBufferDraw();
	// カメラをセットしシーンを設定
	void SetScene();
	// シャドウマップを描画するための前設定をする。
	void PreDrawShadowMap();
	//描画終了メソッド、バリア設定、コマンドリスト実行、フェンスによる同期、コマンドのリセット、画面のスワップによるディスプレイへの描画
	void EndDraw();
	//GPUがコマンドを全て実行完了するまで待ち、CPUと同期する
	void WaitForPreviousFrame();

	//ピクチャロード
	void LoadPictureFromFile(std::wstring filepath, ComPtr<ID3D12Resource>& buff);
};








#endif