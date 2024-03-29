﻿#include"pch.h"
#include"DX12Application.h"
#include"PmdData.h"
#include"Win32Application.h"
#include"Utility.h"
#include"SSAO.h"

using namespace::DirectX;

//ウィンドウサイズの初期値を入れる。レンダーターゲット数を入れる。
DX12Application::DX12Application(UINT width, UINT height) :
	window_width(width),
	window_height(height),
	_backBuffers(buffer_count),parallelLightVec(1,-1,1)
{
}

//初期化、パイプラインの初期化とアセット類のロードを分ける
//シングルトンを実現する。
DX12Application* DX12Application::Instance(UINT width, UINT height)
{
	static DX12Application app{width, height};
	return &app;
}

void DX12Application::OnInit(const HWND& hwnd)
{
	if (FAILED(InitializeDXGIDevice())) {
		assert(0);
		return;
	}
	if (FAILED(InitializeCommands())) {
		assert(0);
		return;
	}
	if (FAILED(CreateSwapChain(hwnd))) {
		assert(0);
		return;
	}
	if (FAILED(CreateFinalRenderTargets())) {
		assert(0);
		return;
	}

	if (FAILED(CreateSceneView())) {
		assert(0);
		return;
	}
	
	ssao = SSAO(_dev);
}

//パイプラインに必要なオブジェクトの生成、初期化を行う
void DX12Application::LoadPipeline()
{
	CreateDepthStencilView();
	CreateSceneView();

}

//DirectX12が終了するときコマンドが全て実行されている確認
void DX12Application::OnDestroy()
{
	WaitForPreviousFrame();

	CloseHandle(_fenceevent);
}


HRESULT DX12Application::InitializeDXGIDevice()
{
	auto result = CoInitializeEx(0, COINIT_MULTITHREADED);

#ifdef _DEBUG
	//デバッグレイヤーの有効化をする

	ID3D12Debug* debugLayer;
	result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));

	debugLayer->EnableDebugLayer(); //デバッグレイヤーの有効化
	debugLayer->Release(); //インターフェイスの解放

#endif

	D3D_FEATURE_LEVEL levels[] =
	{
		//D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	//Directx3Dデバイス生成
	//列挙型levelsを走査し作成可能なバージョンのDirectX12のデバイスを作る
	D3D_FEATURE_LEVEL featurelevel;
	for (auto lv : levels)
	{
		if (D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(_dev.ReleaseAndGetAddressOf())) == S_OK)
		{
			featurelevel = lv;
			break;
		}
	}
	if (_dev == nullptr) {
		return E_FAIL;
	}

	//DXGIFactory生成
#ifdef _DEBUG
	result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(_dxgiFactory.ReleaseAndGetAddressOf()));
#else
	result = CreateDXGIFactory1(IID_PPV_ARGS(_dxgiFactory.ReleaseAndGetAddressOf()));
#endif

	//DebugDevice生成
#ifdef _DEBUG
	result = _dev->QueryInterface(&debugDevice);
#endif

	return result;
}

HRESULT DX12Application::InitializeCommands()
{
	//コマンドアロケーターの生成
	auto result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(_cmdAllocator.ReleaseAndGetAddressOf()));

	//コマンドリストの生成
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		_cmdAllocator.Get(), nullptr, IID_PPV_ARGS(_cmdList.ReleaseAndGetAddressOf()));

	//コマンドキューの設定および生成
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; //タイムアウトなし
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL; //プライオリティ指定なし
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; //コマンドリストと同じタイプ
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(_cmdQueue.ReleaseAndGetAddressOf()));

	return result;
}

HRESULT DX12Application::CreateSwapChain(const HWND& hwnd)
{
	//スワップチェーンの設定および生成
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = window_width;
	swapchainDesc.Height = window_height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = buffer_count;
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ComPtr<IDXGISwapChain1> swapChain;
	auto result = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue.Get(),
		hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		swapChain.ReleaseAndGetAddressOf());
	swapChain.As(&_swapchain);

	return result;
}

HRESULT DX12Application::CreateFinalRenderTargets()
{

	//RTVのディスクリプタヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = buffer_count;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	auto result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(rtvHeaps.ReleaseAndGetAddressOf()));

	//レンダーターゲットビュー
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	//スワップチェーンとディスクリプタを紐づけレンダーターゲットビューを生成する。
	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = _swapchain->GetDesc(&swcDesc);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();

	for (int idx = 0; idx < static_cast<int>(swcDesc.BufferCount); ++idx) {
		result = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));  //_backBufferにスワップチェーン上のバックバッファのメモリを入れる
		_dev->CreateRenderTargetView(_backBuffers[idx].Get(), &rtvDesc, handle);       //バッファの数生成する
		handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);  //ポインタをレンダービューの大きさ分ずらす
	}


	//フェンスの生成
	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_fence.ReleaseAndGetAddressOf()));

	//ビューポートの設定、生成
	viewport.Width = static_cast<float>(window_width);
	viewport.Height = static_cast<float>(window_height);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;

	//シザー矩形の設定、生成
	scissorrect.top = 0;
	scissorrect.left = 0;
	scissorrect.right = scissorrect.left + window_width;
	scissorrect.bottom = scissorrect.top + window_height;

	return result;
}


//GPUがコマンドを全て実行完了するまで待ち、CPUと同期する
void DX12Application::WaitForPreviousFrame()
{
	//命令の完了を待ち、チェック
	//TODO: このSignal()メソッドによる実装は単純なので他の方法を考える
	_cmdQueue->Signal(_fence.Get(), ++_fenceVal);
	if (_fence->GetCompletedValue() != _fenceVal)
	{
		//イベントハンドルの取得
		auto event = CreateEvent(nullptr, false, false, nullptr);

		_fence->SetEventOnCompletion(_fenceVal, event);

		//イベントが発生するまで待ち続ける
		WaitForSingleObject(event, INFINITE);

		CloseHandle(event);
	}
}

void DX12Application::LoadPictureFromFile(std::wstring filepath, ComPtr<ID3D12Resource>& buff)
{
	TexMetadata metadata = {};
	ScratchImage scratchImg = {};
	HRESULT result = S_OK;

	result = LoadFromWICFile(filepath.c_str(),
		WIC_FLAGS_NONE,
		&metadata,
		scratchImg);
	if (FAILED(result)) {
		assert(0);
		return;
	}

	auto img = scratchImg.GetImage(0, 0, 0);

	D3D12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	D3D12_RESOURCE_DESC resdesc = CD3DX12_RESOURCE_DESC::Tex2D(img->format, img->width, img->height);

	result = _dev->CreateCommittedResource(&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(buff.ReleaseAndGetAddressOf()));

	heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc =	CD3DX12_RESOURCE_DESC::Buffer(AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * img->height);

	ComPtr<ID3D12Resource> internalBuffer = nullptr;
	result = _dev->CreateCommittedResource(&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(internalBuffer.ReleaseAndGetAddressOf()));
	if (FAILED(result)) {
		assert(0);
		return;
	}
	uint8_t* mappedInternal = nullptr;
	internalBuffer->Map(0, nullptr, (void**)&mappedInternal);
	auto address = img->pixels;
	uint32_t height = img->height;
	for (int i = 0; i < height; ++i)
	{
		std::copy_n(address, img->rowPitch, mappedInternal);
		address += img->rowPitch;
		mappedInternal += AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	}
	internalBuffer->Unmap(0, nullptr);

	D3D12_TEXTURE_COPY_LOCATION src = {}, dst = {};
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.pResource = internalBuffer.Get();
	src.PlacedFootprint.Offset = 0;
	src.PlacedFootprint.Footprint.Width = img->width;
	src.PlacedFootprint.Footprint.Height = img->height;
	src.PlacedFootprint.Footprint.RowPitch = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	src.PlacedFootprint.Footprint.Depth = metadata.depth;
	src.PlacedFootprint.Footprint.Format = img->format;

	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;
	dst.pResource = buff.Get();

	_cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	auto BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
		buff.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);

	_cmdList->ResourceBarrier(1, &BarrierDesc);
	_cmdList->Close();
	ID3D12CommandList* cmds[] = { _cmdList.Get() };
	_cmdQueue->ExecuteCommandLists(1, cmds);
	WaitForPreviousFrame();
	_cmdAllocator->Reset();
	_cmdList->Reset(_cmdAllocator.Get(), nullptr);
}


//深度ビュー作成メソッド
HRESULT DX12Application::CreateDepthStencilView()
{

	//深度バッファーの作成

	//深度バッファーディスクリプタの設定
	D3D12_RESOURCE_DESC depthResourceDescriptor = {};
	depthResourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResourceDescriptor.Width = window_width;
	depthResourceDescriptor.Height = window_height;
	depthResourceDescriptor.DepthOrArraySize = 1;
	depthResourceDescriptor.Format = DXGI_FORMAT_R32_TYPELESS;
	depthResourceDescriptor.SampleDesc.Count = 1;
	depthResourceDescriptor.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	//深度バッファーヒープの設定
	D3D12_HEAP_PROPERTIES depthHeapProperty = {};
	depthHeapProperty.Type = D3D12_HEAP_TYPE_DEFAULT;
	depthHeapProperty.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	depthHeapProperty.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	//クリアバリューの設定
	D3D12_CLEAR_VALUE depthClearValue = {};
	depthClearValue.DepthStencil.Depth = 1.0f;  //　深さを最大値でクリア
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT; //32bit floatでクリア

	//深度バッファーの作成
	auto result = _dev->CreateCommittedResource(
		&depthHeapProperty,
		D3D12_HEAP_FLAG_NONE,
		&depthResourceDescriptor,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(depthBuffer.ReleaseAndGetAddressOf())
	);

	depthResourceDescriptor.Width = shadowDifinition;
	depthResourceDescriptor.Height = shadowDifinition;

	//シャドウマップ用バッファーの作成
	result = _dev->CreateCommittedResource(
		&depthHeapProperty,
		D3D12_HEAP_FLAG_NONE,
		&depthResourceDescriptor,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(shadowMapBuffer.ReleaseAndGetAddressOf())
	);

	//深度のためのディスクリプタヒープの作成
	//ライトデプスマップも入れる
	D3D12_DESCRIPTOR_HEAP_DESC depthStencilViewHeapDescriptor = {};
	depthStencilViewHeapDescriptor.NumDescriptors = 2;
	depthStencilViewHeapDescriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	result = _dev->CreateDescriptorHeap(&depthStencilViewHeapDescriptor, IID_PPV_ARGS(dsvHeaps.ReleaseAndGetAddressOf()));

	//深度ビューの作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDescriptor = {};
	dsvDescriptor.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDescriptor.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDescriptor.Flags = D3D12_DSV_FLAG_NONE;

	auto handle = dsvHeaps->GetCPUDescriptorHandleForHeapStart();

	//通常デプス
	_dev->CreateDepthStencilView(
		depthBuffer.Get(),
		&dsvDescriptor,
		handle
	);

	handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	//ライトデプス
	_dev->CreateDepthStencilView(
		shadowMapBuffer.Get(),
		&dsvDescriptor,
		handle
	);

	//デプスバッファのSRVをそれぞれ作成
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&depthSRVHeaps));

	D3D12_SHADER_RESOURCE_VIEW_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R32_FLOAT;
	resDesc.Texture2D.MipLevels = 1;
	resDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	resDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	handle = depthSRVHeaps->GetCPUDescriptorHandleForHeapStart();

	//通常デプスSRV
	_dev->CreateShaderResourceView(depthBuffer.Get(), &resDesc, handle);

	//ライトデプスSRV
	handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	_dev->CreateShaderResourceView(shadowMapBuffer.Get(), &resDesc, handle);

	return result;
}


//シーンビュー作成メソッド
//引数を与え、ビュー行列、プロジェクション行列の値を変化させるようにしても良い
HRESULT DX12Application::CreateSceneView()
{
	//ワールド行列、ビュー行列、プロジェクション行列を計算し乗算していく

	XMFLOAT3 eye(0, 15, -25);
	XMFLOAT3 target(0, 10, 0); // eye座標とtarget座標から視線ベクトルを作る
	XMFLOAT3 up(0, 1, 0);

	auto eyePos = XMLoadFloat3(&eye);
	auto targetPos = XMLoadFloat3(&target);
	auto upPos = XMLoadFloat3(&up);

	viewMat = XMMatrixLookAtLH(eyePos,targetPos,upPos);

	projMat = XMMatrixPerspectiveFovLH(
		XM_PIDIV4,
		static_cast<float>(window_width) / static_cast<float>(window_height),
		0.05f,
		100.0f
	);

	//定数バッファーの作成
	auto constHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto constHeapDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(SceneMatrix) + 0xff) & ~0xff);

	_dev->CreateCommittedResource(
		&constHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&constHeapDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(sceneMatrixConstBuff.ReleaseAndGetAddressOf())
	);

	//マップによってバッファーへ行列データを転送
	auto result = sceneMatrixConstBuff->Map(0, nullptr, (void**)&mapTransform);
	//mapMatrix->world = worldMat;
	mapTransform->view = viewMat;
	mapTransform->projection = projMat;
	mapTransform->eye = eye;

	//影行列のセット及びmapTransformへの適用
	XMFLOAT4 planeVec(0, 1, 0, 0);
	mapTransform->shadow = XMMatrixShadow(XMLoadFloat4(&planeVec), -XMLoadFloat3(&parallelLightVec));
	//シャドウマップ用の光源ビューの設定
	//lightの位置はカメラ位置とターゲット位置の距離とライトの位置とターゲット位置が同じになるように設置する。
	float armLength = XMVector3Length(XMVectorSubtract(targetPos, eyePos)).m128_f32[0];
	XMVECTOR lightPos = targetPos + XMVector3Normalize(-XMLoadFloat3(&parallelLightVec))
		* armLength;
	lightMat = XMMatrixLookAtLH(lightPos, targetPos, upPos)
		* XMMatrixOrthographicLH(40, 40, 1.0f, 100.0f);
	mapTransform->lightCamera = lightMat;

	//定数バッファービューの作成のための設定
	//行列用定数バッファービュー用のディスクリプタヒープの作成
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	//シェーダーから見えるように
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = 1;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	result = _dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(matrixCsvHeaps.ReleaseAndGetAddressOf()));

	auto matrixHeaphandle = matrixCsvHeaps->GetCPUDescriptorHandleForHeapStart();

	D3D12_CONSTANT_BUFFER_VIEW_DESC constBufferViewDesc = {};
	constBufferViewDesc.BufferLocation = sceneMatrixConstBuff->GetGPUVirtualAddress();
	constBufferViewDesc.SizeInBytes = static_cast<UINT>(sceneMatrixConstBuff->GetDesc().Width);

	//定数バッファービューの作成
	_dev->CreateConstantBufferView(
		&constBufferViewDesc,
		matrixHeaphandle
	);

	return result;
}

//描画開始メソッド、レンダーターゲット、バリア、深度ビュー、ビューポートのコマンド追加
void DX12Application::SetBackBufferToRTV()
{
	//レンダーターゲットをバックバッファーにセット
	auto bbIdx = _swapchain->GetCurrentBackBufferIndex();
	//リソースバリアの設定
	//リソースバリアをバックバッファーリソースに指定する
	//ヘルパー構造体を使用
	auto BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
		_backBuffers[bbIdx].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	_cmdList->ResourceBarrier(1, &BarrierDesc);



	//レンダーターゲットをバックバッファにセット 
	auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//画面カラーをクリア
	float clearColor[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);


	//深度を設定
	auto dsvH = dsvHeaps->GetCPUDescriptorHandleForHeapStart();
	_cmdList->OMSetRenderTargets(1, &rtvH, true, &dsvH);
	//深度バッファークリア
	//_cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//ビューポート、シザー矩形をセット
	_cmdList->RSSetViewports(1, &viewport);
	_cmdList->RSSetScissorRects(1, &scissorrect);
}

void DX12Application::EndBackBufferDraw()
{

		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();
		auto BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
			_backBuffers[bbIdx].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		);
		_cmdList->ResourceBarrier(1, &BarrierDesc);
}

//カメラをセットしシーンを設定
void DX12Application::SetScene()
{
	//シーン行列定数バッファーをセット
	ID3D12DescriptorHeap* matrixHeaps[] = { matrixCsvHeaps.Get() };
	_cmdList->SetDescriptorHeaps(1, matrixHeaps);
	//ルートパラメタとシーン行列定数ヒープのアドレスの関連付け
	_cmdList->SetGraphicsRootDescriptorTable(0, matrixCsvHeaps->GetGPUDescriptorHandleForHeapStart());
}

//DSVヒープのみをRTVとして設定、DSVクリア、ビューポートの設定
void DX12Application::PreDrawShadowMap()
{
	auto handle = dsvHeaps->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	_cmdList->OMSetRenderTargets(0, nullptr, false, &handle);

	_cmdList->ClearDepthStencilView(handle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	D3D12_VIEWPORT vp = CD3DX12_VIEWPORT(0.0f, 0.0f, shadowDifinition, shadowDifinition);
	_cmdList->RSSetViewports(1, &vp);//ビューポート

	CD3DX12_RECT rc(0, 0, shadowDifinition, shadowDifinition);
	_cmdList->RSSetScissorRects(1, &rc);//シザー(切り抜き)矩形
}

//描画終了メソッド、バリア設定、コマンドリスト実行、フェンスによる同期、コマンドのリセット、画面のスワップによるディスプレイへの描画
void DX12Application::EndDraw()
{


	_cmdList->Close();


	//コマンドリストの命令の実行
	ID3D12CommandList* cmdlists[] = { _cmdList.Get() };
	_cmdQueue->ExecuteCommandLists(1, cmdlists);

	//GPUのコマンド実行の同期を待つ
	WaitForPreviousFrame();

	HRESULT result = _cmdAllocator->Reset(); //キューのリセット
	result = _cmdList->Reset(_cmdAllocator.Get(), nullptr); //再びコマンドリストをためる準備

	//画面をスワップし描画する
	_swapchain->Present(1, 0);
}
