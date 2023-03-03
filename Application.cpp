#include"pch.h"
#include"Application.h"
#include"PmdManager.h"
#include"Win32Application.h"

using namespace::DirectX;

//ウィンドウサイズの初期値を入れる。レンダーターゲット数を入れる。
DX12Application::DX12Application(UINT width, UINT height) :
	window_width(width),
	window_height(height),
	_backBuffers(buffer_count)
{
}

//初期化、パイプラインの初期化とアセット類のロードを分ける
//シングルトンを実現する。
DX12Application* DX12Application::Instance(UINT width, UINT height)
{

	static DX12Application app{width, height};
	return &app;
}

void DX12Application::OnInit()
{
	//DirectXTexライブラリのテクスチャロードのための準備


	LoadPipeline();
	LoadAssets();
}


//パイプラインに必要なオブジェクトの生成、初期化を行う
//TODO: オブジェクトの初期化はThrowIfFailed()関数に入れる
void DX12Application::LoadPipeline()
{
	auto result = CoInitializeEx(0, COINIT_MULTITHREADED);

#ifdef _DEBUG
	//デバッグレイヤーの有効化をする

	ID3D12Debug* debugLayer ;
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
		return;
	}

	//DXGIFactory生成
#ifdef _DEBUG
	result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(_dxgiFactory.ReleaseAndGetAddressOf()));
#else
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(_dxgiFactory.ReleaseAndGetAddressOf()));
#endif

	//DebugDevice生成
#ifdef _DEBUG
	result = _dev->QueryInterface(&debugDevice);
#endif

	//コマンドアロケーターの生成
	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
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
	result = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue.Get(),
		Win32Application::GetHwnd(),
		&swapchainDesc,
		nullptr,
		nullptr,
		swapChain.ReleaseAndGetAddressOf());
	swapChain.As(&_swapchain);

	//RTVのディスクリプタヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = buffer_count;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(rtvHeaps.ReleaseAndGetAddressOf()));

	//　SRGBレンダーターゲットビュー
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	//スワップチェーンとディスクリプタを紐づけレンダーターゲットビューを生成する。
	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = _swapchain->GetDesc(&swcDesc);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();

	for (int idx = 0; idx < swcDesc.BufferCount; ++idx) {
		result = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));  //_backBufferにスワップチェーン上のバックバッファのメモリを入れる
		_dev->CreateRenderTargetView(_backBuffers[idx], &rtvDesc, handle);       //バッファの数生成する
		handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);  //ポインタをレンダービューの大きさ分ずらす
	}

	//フェンスの生成
	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_fence.ReleaseAndGetAddressOf()));

	//ビューポートの設定、生成
	viewport.Width = window_width;
	viewport.Height = window_height;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;

	//シザー矩形の設定、生成
	scissorrect.top = 0;
	scissorrect.left = 0;
	scissorrect.right = scissorrect.left + window_width;
	scissorrect.bottom = scissorrect.top + window_height;

}


//アセットのロード、現状は頂点、頂点インデックス、シェーダー、PSO、ルートシグネチャ等
void DX12Application::LoadAssets()
{
	
	//PMDデータのロード
	std::string strModelPath = "Model/初音ミクmetal.pmd";
	//std::string strModelPath = "Model/鏡音レン.pmd";
	//std::string strModelPath = "Model/鏡音リン.pmd";
	//std::string strModelPath = "Model/巡音ルカ.pmd";
	//std::string strModelPath = "Model/弱音ハク.pmd";
	//std::string strModelPath = "Model/初音ミク.pmd";

	
	pmdData.loadPmdData(strModelPath);






	//頂点バッファーの生成
	D3D12_HEAP_PROPERTIES heapprop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC resourcedesc = CD3DX12_RESOURCE_DESC::Buffer(pmdData.vertices.size());

	vertBuff = nullptr;
	auto result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resourcedesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(vertBuff.ReleaseAndGetAddressOf())
	);

	//MapメソッドでビデオメモリvertBuff上に頂点を書き込む
	unsigned char* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap); //vertMap上にvertBuffの仮想メモリを置く
	std::copy(std::begin(pmdData.vertices), std::end(pmdData.vertices), vertMap);
	vertBuff->Unmap(0, nullptr);  //仮想メモリを解除

	//頂点バッファービューの生成
	vbView = {};
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = pmdData.vertices.size();
	vbView.StrideInBytes = pmdData.pmdvertex_size;

	//インデックスバッファーの生成
	idxBuff = nullptr;

	resourcedesc = CD3DX12_RESOURCE_DESC::Buffer(pmdData.indices.size() * sizeof(pmdData.indices[0]));

	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resourcedesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(idxBuff.ReleaseAndGetAddressOf())
	);

	unsigned short* mappedIdx = nullptr;
	idxBuff->Map(0, nullptr, (void**)&mappedIdx);

	std::copy(std::begin(pmdData.indices), std::end(pmdData.indices), mappedIdx);
	idxBuff->Unmap(0, nullptr);

	//インデックスバッファービューの作成
	ibView = {};

	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = static_cast<UINT>(pmdData.indices.size() * sizeof(pmdData.indices[0]));



	
	//マテリアルの数だけテクスチャをロードする。対応するテクスチャがなければnullptrを入れる。
	//テクスチャ名にセパレーターがあれば分離し、適切な名前を入れる
	textureResource.resize(pmdData.materialNum);
	sphResources.resize(pmdData.materialNum);
	spaResources.resize(pmdData.materialNum);

	for (int i = 0; i < pmdData.materialDatas.size(); ++i)
	{
		std::string texFileName = pmdData.materialDatas[i].additional.texPath;
		std::string sphFileName = {};
		std::string spaFileName = {};

		if (texFileName.size() == 0)
		{
			textureResource[i] = nullptr;
			continue;
		}

		{
			auto namepair = SplitFileName(texFileName, '*');
			if (GetExtension(namepair.second) == "sph" )
			{
				sphFileName = namepair.second;
				texFileName = namepair.first;
			}
			else if (GetExtension(namepair.second) == "spa")
			{
				spaFileName = namepair.second;
				texFileName = namepair.first;
			}
			else
			{
				texFileName = namepair.second;
			}
		}

		auto texFilePath = GetTexturePathFromModelAndTexPath(
			strModelPath,
			texFileName.c_str());

		auto sphFilePath = GetTexturePathFromModelAndTexPath(
			strModelPath,
			sphFileName.c_str());
		auto spaFilePath = GetTexturePathFromModelAndTexPath(
			strModelPath,
			spaFileName.c_str());

		textureResource[i] = LoadTextureFromFile(texFilePath);
		sphResources[i] = LoadTextureFromFile(sphFilePath);
		spaResources[i] = LoadTextureFromFile(spaFilePath);

	}

	//トゥーンシェーダーのためのカラールックアップテーブルのロード
	toonResources.resize(pmdData.materialDatas.size());

	for (int i = 0; i < pmdData.materialDatas.size(); ++i)
	{
		std::string toonFilePath = "toon/";
		char toonFileName[16];

		sprintf_s(
			toonFileName,
			16,
			"toon%02d.bmp",
			pmdData.materialDatas[i].additional.toonIdx + 1
		);

		toonFilePath += toonFileName;

		toonResources[i] = LoadTextureFromFile(toonFilePath);
	}

	//白、黒、グラデーションのテクスチャの作成
	whiteTex = CreateWhiteTexture();
	blackTex = CreateBlackTexture();
	gradTex = CreateGradationTexture();

	//マテリアルバッファーの作成、バッファーサイズを256バイトでアライメントする
	auto materialBuffSize = sizeof(MaterialForHlsl);
	materialBuffSize = (materialBuffSize + 0xff) & ~0xff;

	auto materialHeapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto materialResourceDescriptor = CD3DX12_RESOURCE_DESC::Buffer(materialBuffSize * pmdData.materialNum);

	result = _dev->CreateCommittedResource(
		&materialHeapProperty,
		D3D12_HEAP_FLAG_NONE,
		&materialResourceDescriptor,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(materialBuff.ReleaseAndGetAddressOf())
	);

	result = materialBuff->Map(0, nullptr, (void**)&mapMaterial);

	//char*をmaterialForHlsl*型に変換
	for (auto& m : pmdData.materialDatas) {
		*reinterpret_cast<MaterialForHlsl*>(mapMaterial) = m.material;
		mapMaterial += materialBuffSize;
	}
	materialBuff->Unmap(0, nullptr);

	//マテリアルとテクスチャ用のディスクリプタヒープの作成。
	D3D12_DESCRIPTOR_HEAP_DESC DescHeapDesc = {};
	DescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	DescHeapDesc.NodeMask = 0;
	DescHeapDesc.NumDescriptors = pmdData.materialNum * 5;
	DescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	result = _dev->CreateDescriptorHeap(
		&DescHeapDesc, IID_PPV_ARGS(materialDescHeap.ReleaseAndGetAddressOf())
	);

	//シェーダーリソースビューディスクリプタの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	//マテリアル用CBV、SRVの作成、マテリアルの数だけ作る。
	D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = {};
	matCBVDesc.BufferLocation = materialBuff->GetGPUVirtualAddress();
	matCBVDesc.SizeInBytes = materialBuffSize;

	auto matDescHeapHead = materialDescHeap->GetCPUDescriptorHandleForHeapStart(); //先頭を記録
	auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//以下実際にCBV、SRVを作る。
	for (int i = 0; i < pmdData.materialNum; ++i)
	{
		_dev->CreateConstantBufferView(&matCBVDesc, matDescHeapHead);
		matDescHeapHead.ptr += inc;
		matCBVDesc.BufferLocation += materialBuffSize;

		if (textureResource[i] != nullptr)		//テクスチャがあればそのテクスチャ、なければ白テクスチャを設定
		{
			srvDesc.Format = textureResource[i]->GetDesc().Format;
			_dev->CreateShaderResourceView(
				textureResource[i].Get(),
				&srvDesc,
				matDescHeapHead
			);
		}
		else
		{
			srvDesc.Format = whiteTex->GetDesc().Format;
			_dev->CreateShaderResourceView(
				whiteTex.Get(),
				&srvDesc,
				matDescHeapHead
			);

		}
		matDescHeapHead.ptr += inc;

		if (sphResources[i] != nullptr)		//SPHがあればそのSPH、なければ白テクスチャを設定
		{
			srvDesc.Format = sphResources[i]->GetDesc().Format;
			_dev->CreateShaderResourceView(
				sphResources[i].Get(),
				&srvDesc,
				matDescHeapHead
			);
		}
		else
		{
			srvDesc.Format = whiteTex->GetDesc().Format;
			_dev->CreateShaderResourceView(
				whiteTex.Get(),
				&srvDesc,
				matDescHeapHead
			);

		}
		matDescHeapHead.ptr += inc;

		if (spaResources[i] != nullptr)		//SPAがあればそのSPA、なければ黒テクスチャを設定
		{
			srvDesc.Format = spaResources[i]->GetDesc().Format;
			_dev->CreateShaderResourceView(
				spaResources[i].Get(),
				&srvDesc,
				matDescHeapHead
			);
		}
		else
		{
			srvDesc.Format = blackTex->GetDesc().Format;
			_dev->CreateShaderResourceView(
				blackTex.Get(),
				&srvDesc,
				matDescHeapHead
			);

		}

		matDescHeapHead.ptr += inc;

		if (toonResources[i] != nullptr)	//トゥーンリソースがあればトゥーン用、なければグラデーションテクスチャを設定
		{
			srvDesc.Format = toonResources[i]->GetDesc().Format;
			_dev->CreateShaderResourceView(
				toonResources[i].Get(),
				&srvDesc,
				matDescHeapHead
			);
		}
		else
		{
			srvDesc.Format = gradTex->GetDesc().Format;
			_dev->CreateShaderResourceView(
				gradTex.Get(),
				&srvDesc,
				matDescHeapHead
			);

		}
		matDescHeapHead.ptr += inc;

	}

	//ワールド行列、ビュー行列、プロジェクション行列を計算し乗算していく
	worldMat = XMMatrixRotationY(0);

	XMFLOAT3 eye(0, 15, -15); 
	XMFLOAT3 target(0, 15, 0); // eye座標とtarget座標から視線ベクトルを作る
	XMFLOAT3 up(0, 1, 0);

	viewMat = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));

	projMat = XMMatrixPerspectiveFovLH(
		XM_PIDIV4,
		static_cast<float>(window_width) / static_cast<float>(window_height),
		1.0f,
		100.0f
	);


	CreateSceneView();
	CreateDepthStencilView();

	//頂点シェーダーオブジェクトの生成
	result = D3DCompileFromFile(
		L"BasicVertexShader.hlsl",  //シェーダー名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, //カレントディレクトリからインクルードする
		"BasicVS", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, //オプションにデバッグと最適化をスキップ
		0,
		&_vsBlob, &errorBlob);

	//エラー発生時の処理
	if (FAILED(result)) {
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("ファイルが見当たりません");
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			::OutputDebugStringA(errstr.c_str());
		}
		exit(1);
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

	//エラー発生時の処理
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
		exit(1);
	}

	//頂点レイアウトの設定
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{//座標情報
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{//法線ベクトル情報
			"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{//uv情報
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
			0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{//ボーン情報
			"BONE_NO", 0, DXGI_FORMAT_R16G16_UINT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{//ボーンウェイト情報
			"WEIGHT", 0, DXGI_FORMAT_R8_UINT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{//輪郭線フラグ情報
			"EDGE_FLG", 0, DXGI_FORMAT_R8_UINT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};


	//パイプラインステートの作成、設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};
	//シェーダーを設定
	gpipeline.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength = _vsBlob->GetBufferSize();
	gpipeline.PS.pShaderBytecode = _psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = _psBlob->GetBufferSize();
	//サンプルマスクとラスタライザーステートを設定
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//ブレンドステートの設定
	gpipeline.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	//ラスタライザーの設定
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpipeline.RasterizerState.MultisampleEnable = false;
	//深度バッファーの設定
	gpipeline.DepthStencilState.DepthEnable = true;
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL; //深度バッファーに深度値を書き込みか否かの指定
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS; //深度値の小さいほうを採用する。　つまり距離が近い法？
	gpipeline.DepthStencilState.StencilEnable = false;
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;
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


	//ルートシグネチャに設定するルートパラメータ及びディスクリプタテーブル、ディスクリプタレンジの設定
	//１つ目は行列用定数バッファービューの設定、２つ目はマテリアル用定数バッファの設定、3つ目はシェーダーリソースビューの設定
	CD3DX12_DESCRIPTOR_RANGE descTblRange[3] = {};
	descTblRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);//CBV b0
	descTblRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);//CBV b1
	descTblRange[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);//SRV t0 - t3

	//ルートパラメーターの設定
	CD3DX12_ROOT_PARAMETER rootparam[2] = {};

	rootparam[0].InitAsDescriptorTable(1, &descTblRange[0]);
	rootparam[1].InitAsDescriptorTable(2, &descTblRange[1]);


	//ルートシグネチャに設定するサンプラーの設定
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc[2] = {};

	samplerDesc[0].Init(0);
	samplerDesc[1].Init(1,D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	//ルートシグネチャの設定、生成
	//ルートシグネチャディスクリプタの設定
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//ディスクリプタテーブルの実体であるルートパラメーターを設定
	rootSignatureDesc.pParameters = &rootparam[0];
	rootSignatureDesc.NumParameters = 2;
	//サンプラーを設定
	rootSignatureDesc.pStaticSamplers = samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 2;

	//ルートシグネチャのBlobの作成
	ID3DBlob* rootSigBlob = nullptr;

	result = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob,
		&errorBlob);

	//ルートシグネチャオブジェクトの作成
	rootsignature = nullptr;
	result = _dev->CreateRootSignature(
		0,
		rootSigBlob->GetBufferPointer(),
		rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(rootsignature.ReleaseAndGetAddressOf()));

	rootSigBlob->Release();

	//グラフィクスPSOオブジェクトの生成
	gpipeline.pRootSignature = rootsignature.Get();
	_pipelinestate = nullptr;
	result = _dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(_pipelinestate.ReleaseAndGetAddressOf()));




}

//フレームによって更新する値を入れる予定
void DX12Application::OnUpdate()
{

	//行列変換用行列をフレーム毎に更新し板ポリゴンがY軸で回転するようにする。
	//angle += 0.01f;
	//worldMat = XMMatrixRotationY(angle);
	//mapMatrix->world = worldMat;
}


//レンダリングする。メインループの内部
void DX12Application::OnRender() 
{
	//コマンドリストに実際に実行するレンダリングコマンドを集める
	PopulateCommandList();

	//コマンドリストの命令の実行
	ID3D12CommandList* cmdlists[] = { _cmdList.Get()};
	_cmdQueue->ExecuteCommandLists(1, cmdlists);

	//GPUのコマンド実行の同期を待つ
	WaitForPreviousFrame();

	HRESULT result = _cmdAllocator->Reset(); //キューのリセット
	result = _cmdList->Reset(_cmdAllocator.Get(), nullptr); //再びコマンドリストをためる準備

	//画面をスワップし描画する
	_swapchain->Present(1, 0);
}

//DirectX12が終了するときコマンドが全て実行されている確認
void DX12Application::OnDestroy() 
{
	WaitForPreviousFrame();

	CloseHandle(_fenceevent);
}


//コマンドリストに実際に実行するコマンドを追加
void DX12Application::PopulateCommandList()
{





	//auto result = _cmdAllocator->Reset();  //コマンドアロケーターのリセット。

	BeginDraw();

	//描画命令
	//PSO、RootSignature,Primitive topologyのセット
	_cmdList->SetPipelineState(_pipelinestate.Get());
	_cmdList->SetGraphicsRootSignature(rootsignature.Get());
	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	SetScene();

	//頂点バッファーのセット
	_cmdList->IASetVertexBuffers(0, 1, &vbView);
	//インデックスバッファーのセット
	_cmdList->IASetIndexBuffer(&ibView);




	//マテリアルディスクリプタヒープのセット
	ID3D12DescriptorHeap* ppHeaps1[] = { materialDescHeap.Get() };
	_cmdList->SetDescriptorHeaps(1, ppHeaps1);

	//マテリアルのディスクリプタテーブルのセットとそれに対応したインデッックスを更新しながら描画していく。
	//その上さらにテクスチャ、sph、spa、トゥーンテクスチャも描画していく。
	auto materialH = materialDescHeap->GetGPUDescriptorHandleForHeapStart();
	auto cbvSrvIncSize = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 5;

	unsigned int idxOffset = 0;
	for (auto& m : pmdData.materialDatas)
	{
		_cmdList->  SetGraphicsRootDescriptorTable(1, materialH);

		_cmdList->DrawIndexedInstanced(m.indicesNum, 1, idxOffset, 0, 0);

		materialH.ptr += cbvSrvIncSize;

		idxOffset += m.indicesNum;
	}

	//リソースバリアの状態の設定
	auto bbIdx = _swapchain->GetCurrentBackBufferIndex();
	auto BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
		_backBuffers[bbIdx],
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);
	_cmdList->ResourceBarrier(1, &BarrierDesc);

	

	_cmdList->Close();
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
	depthResourceDescriptor.Format = DXGI_FORMAT_D32_FLOAT;
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


	//深度のためのディスクリプタヒープの作成
	D3D12_DESCRIPTOR_HEAP_DESC depthStencilViewHeapDescriptor = {};
	depthStencilViewHeapDescriptor.NumDescriptors = 1;
	depthStencilViewHeapDescriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	result = _dev->CreateDescriptorHeap(&depthStencilViewHeapDescriptor, IID_PPV_ARGS(dsvHeaps.ReleaseAndGetAddressOf()));

	//深度ビューの作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDescriptor = {};
	dsvDescriptor.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDescriptor.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDescriptor.Flags = D3D12_DSV_FLAG_NONE;
	_dev->CreateDepthStencilView(
		depthBuffer.Get(),
		&dsvDescriptor,
		dsvHeaps->GetCPUDescriptorHandleForHeapStart()
	);

	return result;
}


//シーンビュー作成メソッド
//引数を与え、ビュー行列、プロジェクション行列の値を変化させるようにしても良い
HRESULT DX12Application::CreateSceneView()
{
	//ワールド行列、ビュー行列、プロジェクション行列を計算し乗算していく
	worldMat = XMMatrixRotationY(0);

	XMFLOAT3 eye(0, 15, -15);
	XMFLOAT3 target(0, 15, 0); // eye座標とtarget座標から視線ベクトルを作る
	XMFLOAT3 up(0, 1, 0);

	viewMat = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));

	projMat = XMMatrixPerspectiveFovLH(
		XM_PIDIV4,
		static_cast<float>(window_width) / static_cast<float>(window_height),
		1.0f,
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
	auto result = sceneMatrixConstBuff->Map(0, nullptr, (void**)&mapMatrix);
	mapMatrix->world = worldMat;
	mapMatrix->view = viewMat;
	mapMatrix->projection = projMat;
	mapMatrix->eye = eye;

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
	constBufferViewDesc.SizeInBytes = sceneMatrixConstBuff->GetDesc().Width;

	//定数バッファービューの作成
	_dev->CreateConstantBufferView(
		&constBufferViewDesc,
		matrixHeaphandle
	);

	return result;
}

//描画開始メソッド、レンダーターゲット、バリア、深度ビュー、ビューポートのコマンド追加
void DX12Application::BeginDraw()
{
	//レンダーターゲットをバックバッファーにセット
	auto bbIdx = _swapchain->GetCurrentBackBufferIndex();
	//リソースバリアの設定
	//リソースバリアをバックバッファーリソースに指定する
	//ヘルパー構造体を使用
	auto BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
		_backBuffers[bbIdx],
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	_cmdList->ResourceBarrier(1, &BarrierDesc);



	//レンダーターゲットをバックバッファにセット 
	auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//画面カラーをクリア
	float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);


	//深度を設定
	auto dsvH = dsvHeaps->GetCPUDescriptorHandleForHeapStart();
	_cmdList->OMSetRenderTargets(1, &rtvH, true, &dsvH);
	//深度バッファークリア
	_cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//ビューポート、シザー矩形をセット
	_cmdList->RSSetViewports(1, &viewport);
	_cmdList->RSSetScissorRects(1, &scissorrect);
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

//描画終了メソッド、バリア設定、コマンドリスト実行、フェンスによる同期、コマンドのリセット、画面のスワップによるディスプレイへの描画
void DX12Application::EndDraw()
{
	//リソースバリアの状態の設定
	auto bbIdx = _swapchain->GetCurrentBackBufferIndex();
	auto BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
		_backBuffers[bbIdx],
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);
	_cmdList->ResourceBarrier(1, &BarrierDesc);

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
