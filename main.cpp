#include <Windows.h>
#include <tchar.h>
#include<vector>
#include<string>
#ifdef _DEBUG
#include <iostream>
#endif

#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<d3dcompiler.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;


//将来的にヘッダーファイルへ
D3D_FEATURE_LEVEL levels[] =
{
	D3D_FEATURE_LEVEL_12_2,
	D3D_FEATURE_LEVEL_12_1,
	D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
};



// @brief コンソール画面にフォーマットつき文字列を表示（プレースホルダー)
// @param format フォーマット
// @pram	可変長引数
// @remarks	デバッグ

//デバッグ用関数
void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif
}

//デバッグレイヤーの有効化をする
void EnableDebugLayer()
{
	ID3D12Debug* debugLayer = nullptr;
	auto result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));

	debugLayer->EnableDebugLayer(); //デバッグレイヤーの有効化
	debugLayer->Release(); //インターフェイスの解法
}


//ウィンドウに必要な関数
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// ウィンドウが破壊されたら呼ばれる
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0); //OSに終了を伝える
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}


//main関数　メッセージループが入っている
#ifdef _DEBUG
int main()
{

#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#endif
	//ウィンドウクラスの生成
	WNDCLASSEX w = {};

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure; //コールバック関数の指定
	w.lpszClassName = _T(" DX 12 Sample");
	w.hInstance = GetModuleHandle(nullptr);

	RegisterClassEx(&w); //application class

	int window_width = 1600;
	int window_height = 1000;
	RECT wrc = { 0, 0, window_width, window_height };//ウィンドウサイズの決定

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false); //ウィンドウサイズ補正


	//ウィンドウオブジェクトの生成
	HWND hwnd = CreateWindowW(w.lpszClassName, //クラス名指定
		_T("DX12TEST"),	//タイトルバーの文字
		WS_OVERLAPPEDWINDOW, //タイトルバーとの境界線
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,//ウィンドウ幅
		wrc.bottom - wrc.top,//ウィンドウ高
		nullptr,				//親ウィンドウハンドル
		nullptr,				//メニューハンドル
		w.hInstance,			//呼び出しアプリケーションハンドル
		nullptr);			//追加パラメーター

#ifdef _DEBUG
	//デバッグレイヤーを有効化する
	EnableDebugLayer();
#endif
	//Directx基本オブジェクトの宣言
	ID3D12Device* _dev = nullptr;
	IDXGIFactory6* _dxgiFactory = nullptr;
	IDXGISwapChain4* _swapchain = nullptr;
	ID3D12CommandAllocator* _cmdAllocator = nullptr;
	ID3D12GraphicsCommandList* _cmdList = nullptr;
	ID3D12CommandQueue* _cmdQueue = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	ID3D12DescriptorHeap* rtvHeaps = nullptr;
	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceVal = 0;


	//Directx3Dデバイス生成
	D3D_FEATURE_LEVEL featurelevel;
	for (auto lv : levels)
	{
		if (D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			featurelevel = lv;
			break;
		}
	}
	//DXGIFactory生成
#ifdef _DEBUG
	auto result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory));
#else
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
#endif
	//コマンドアロケーターの生成
	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&_cmdAllocator));
	//コマンドリストの生成
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		_cmdAllocator, nullptr, IID_PPV_ARGS(&_cmdList));
	//コマンドキューの設定および生成
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; //タイムアウトなし
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL; //プライオリティ指定なし
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; //コマンドリストと同じタイプ
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));
	//スワップチェーンの設定および生成
	swapchainDesc.Width = window_width;
	swapchainDesc.Height = window_height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	result = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue,
		hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)&_swapchain);
	//ディスクリプタヒープの設定
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));
	//スワップチェーンとディスクリプタを紐づけレンダーターゲットビューを生成する。
	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = _swapchain->GetDesc(&swcDesc);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	std::vector<ID3D12Resource*> _backBuffers(swcDesc.BufferCount);
	for (int idx = 0; idx < swcDesc.BufferCount; ++idx) {
		result = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));  //_backBufferにスワップチェーン上のバックバッファのメモリを入れる
		_dev->CreateRenderTargetView(_backBuffers[idx], nullptr, handle);       //バッファの数生成する
		handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);  //ポインタをレンダービューの大きさ分ずらす
	}
	//フェンスの生成
	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));


	//ウィンドウ表示
	ShowWindow(hwnd, SW_SHOW);


	//リソースの生成
	// 
	XMFLOAT3 vertices[] = {
		{-0.4f,-0.7f,0.0f} ,//左下
		{-0.4f,0.7f,0.0f} ,//左上
		{0.4f,-0.7f,0.0f} ,//右下
		{0.4f,0.7f,0.0f} ,//右上
	};

	unsigned short indices[] = {
		0, 1, 2,
		2, 1, 3
	};

	//頂点バッファーの生成
	D3D12_HEAP_PROPERTIES heapprop = {}; //頂点のヒープの設定
	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resdesc = {};//頂点のリソースのディスクリプタの設定
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(vertices);
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* vertBuff = nullptr;
	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff)
	);

	//MapメソッドでビデオメモリvertBuff上に頂点を書き込む
	XMFLOAT3* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap); //vertMap上にvertBuffの仮想メモリを置く
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertBuff->Unmap(0, nullptr);  //仮想メモリを解除

	//頂点バッファービューの生成
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeof(vertices);
	vbView.StrideInBytes = sizeof(vertices[0]);

	//インデックスバッファーの生成
	ID3D12Resource* idxBuff = nullptr;

	resdesc.Width = sizeof(indices);

	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&idxBuff)
	);

	unsigned short* mappedIdx = nullptr;
	idxBuff->Map(0, nullptr, (void**)&mappedIdx);

	std::copy(std::begin(indices), std::end(indices), mappedIdx);
	idxBuff->Unmap(0, nullptr);

	//インデックスバッファービューの作成
	D3D12_INDEX_BUFFER_VIEW ibView = {};

	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeof(indices);

	//シェーダーオブジェクトの宣言
	ID3DBlob* _vsBlob = nullptr;
	ID3DBlob* _psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	//頂点シェーダーオブジェクトの生成
	result = D3DCompileFromFile(
		L"BasicVertexShader.hlsl",  //シェーダー名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, //カレントディレクトリからインクルードする
		"BasicVS", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, //オプションにデバッグと最適化をスキップ
		0,
		&_vsBlob, &errorBlob);

	if (FAILED(result)) {
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("ファイルが見当たりません");
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		exit(1);//行儀悪いかな…
	}

	//ピクセルシェーダーオブジェクトの生成
	result = D3DCompileFromFile(
		L"BasicPixelShader.hlsl",  //シェーダー名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, //カレントディレクトリからインクルードする
		"BasicPS", "ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&_psBlob, &errorBlob);

	if (FAILED(result)) {
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("ファイルが見当たりません");
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		exit(1);//行儀悪いかな…
	}
	
	//頂点レイアウトの設定
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};

	//パイプラインステートの作成、設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};
	//ルートジグネチャ、シェーダーを設定
	gpipeline.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength = _vsBlob->GetBufferSize();
	gpipeline.PS.pShaderBytecode = _psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = _psBlob->GetBufferSize();
	//サンプルマスクとラスタライザーステートを設定
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	gpipeline.RasterizerState.MultisampleEnable = false;
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	gpipeline.RasterizerState.DepthClipEnable = true;
	//ブレンドステートの設定
	gpipeline.BlendState.AlphaToCoverageEnable = false;
	gpipeline.BlendState.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;

	gpipeline.RasterizerState.FrontCounterClockwise = false;
	gpipeline.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	gpipeline.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	gpipeline.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	gpipeline.RasterizerState.AntialiasedLineEnable = false;
	gpipeline.RasterizerState.ForcedSampleCount = 0;
	gpipeline.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	gpipeline.DepthStencilState.DepthEnable = false;
	gpipeline.DepthStencilState.StencilEnable = false;

	//入力レイアウトの設定
	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);
	//トライアングルストリップの設定
	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	//プリミティブトポロジの設定
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//レンダーターゲットの設定
	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	//アンチエイリアシングのサンプル数の設定
	gpipeline.SampleDesc.Count = 1;
	gpipeline.SampleDesc.Quality = 0;

	//PSOの設定の終了
	
	//ルートシグネチャの設定、生成
	//ルートシグネチャディスクリプタの設定
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//ルートシグネチャのBlobの作成
	ID3DBlob* rootSigBlob = nullptr;


	result = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob,
		&errorBlob		);

	//ルートシグネチャオブジェクトの作成
	ID3D12RootSignature* rootsignature = nullptr;
	result = _dev->CreateRootSignature(
		0,
		rootSigBlob->GetBufferPointer(),
		rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootsignature));

	rootSigBlob->Release();

	//グラフィクスPSOオブジェクトの生成
	gpipeline.pRootSignature = rootsignature;
	ID3D12PipelineState* _pipelinestate = nullptr;
	result = _dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&_pipelinestate));

	//ビューポートの設定、生成
	D3D12_VIEWPORT viewport = {};

	viewport.Width = window_width;
	viewport.Height = window_height;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;

	//シザー矩形の設定、生成
	D3D12_RECT scissorrect = {};

	scissorrect.top = 0;
	scissorrect.left = 0;
	scissorrect.right = scissorrect.left + window_width;
	scissorrect.bottom = scissorrect.top + window_height;





	//メッセージループの開始
	MSG msg = {};
	int frame = 0;
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//アプリケーションが終わるときmessageがWM_QUITになる
		if (msg.message == WM_QUIT)
		{
			break;
		}

		//DirectXの処理

		//result = _cmdAllocator->Reset();  //コマンドアロケーターのリセット。おそらくいらないのでコメントアウト
		//レンダーターゲットをバックバッファーにセット
		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();
		//リソースバリアの設定
		//リソースバリアをバックバッファーリソースに指定する
		D3D12_RESOURCE_BARRIER BarrierDesc = {};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		BarrierDesc.Transition.pResource = _backBuffers[bbIdx];
		BarrierDesc.Transition.Subresource = 0;
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		_cmdList->ResourceBarrier(1, &BarrierDesc);

		_cmdList->SetPipelineState(_pipelinestate);

		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr); //レンダーターゲットをバックバッファにセット 

		//画面クリア
		float r, g, b;
		r = (float)(0xff & frame >> 16) / 255.0f;
		g = (float)(0xff & frame >> 8) / 255.0f;
		b = (float)(0xff & frame >> 0) / 255.0f;
		float clearColor[] = { r,g,b, 1.0f };// 
		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
		++frame;

		//描画命令
		//PSO、RootSignature,Primitive topologyのセット
		_cmdList->SetGraphicsRootSignature(rootsignature);
		_cmdList->RSSetViewports(1, &viewport);
		_cmdList->RSSetScissorRects(1, &scissorrect);
		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//頂点バッファーのセット
		_cmdList->IASetVertexBuffers(0, 1, &vbView);

		//インデックスバッファーのセット
		_cmdList->IASetIndexBuffer(&ibView);

		//実際の描画命令
		//_cmdList->DrawInstanced(4, 1, 0, 0);
		_cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);


		//リソースバリアの状態の設定
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		_cmdList->ResourceBarrier(1, &BarrierDesc);


		//コマンドリストの命令の実行
		_cmdList->Close();
		ID3D12CommandList* cmdlists[] = { _cmdList };
		_cmdQueue->ExecuteCommandLists(1, cmdlists);


		//命令の完了を待ち、チェック
		_cmdQueue->Signal(_fence, ++_fenceVal);
		if (_fence->GetCompletedValue() != _fenceVal)
		{
			//イベントハンドルの取得
			auto event = CreateEvent(nullptr, false, false, nullptr);

			_fence->SetEventOnCompletion(_fenceVal, event);

			//イベントが発生するまで待ち続ける
			WaitForSingleObject(event, INFINITE);

			CloseHandle(event);
		}

		result = _cmdAllocator->Reset(); //キューのリセット
		result = _cmdList->Reset(_cmdAllocator, nullptr); //再びコマンドリストをためる準備

		//画面のスワップ
		_swapchain->Present(1, 0);


		
	}

	//もうクラスはつかわないので登録解除する
	UnregisterClass(w.lpszClassName, w.hInstance);



	return 0;

}