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
	//DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
	DirectX::XMFLOAT3 eye;
};


class DX12Application
{
private:

	//シングルトンクラスにするためコンストラクタをprivate
	DX12Application(UINT window_width, UINT window_height);
	//ReportLiveDeviceObjectsを呼ぶ
	//~Application() {
	//	debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
	//	debugDevice->Release();
	//};
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
	std::vector<ID3D12Resource*> _backBuffers;

	//アセットの宣言
	ComPtr<ID3D12DescriptorHeap> matrixCsvHeaps = nullptr;
	ComPtr<ID3D12Resource> sceneMatrixConstBuff = nullptr;

	ComPtr<ID3D12Resource> depthBuffer = nullptr;
	ComPtr<ID3D12DescriptorHeap> dsvHeaps = nullptr;

	//行列アセット
	DirectX::XMMATRIX viewMat;
	DirectX::XMMATRIX projMat;
	SceneMatrix* mapMatrix = nullptr;
	float angle = 0.0f;


	//同期オブジェクトの宣言


	//デバッグオブジェクトの宣言
	ID3D12DebugDevice* debugDevice;


	void PopulateCommandList();
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
	void OnUpdate();
	void OnRender();
	void OnDestroy();
	void LoadPipeline();
	void LoadAssets();
	//シングルトンパターンのためstaticの生成メソッド
	static DX12Application* Instance(UINT width, UINT height);
	//描画開始メソッド、レンダーターゲット、バリア、深度ビュー、ビューポートのコマンド追加
	void BeginDraw();
	// カメラをセットしシーンを設定
	void SetScene();
	//描画終了メソッド、バリア設定、コマンドリスト実行、フェンスによる同期、コマンドのリセット、画面のスワップによるディスプレイへの描画
	void EndDraw();
	//GPUがコマンドを全て実行完了するまで待ち、CPUと同期する
	void WaitForPreviousFrame();
};








#endif