#include "pch.h"
#include "PmdRenderer.h"

PmdRenderer::PmdRenderer(DX12Application* app)
{
	CreateRootSignature(app);
	CreateGraphicsPipelineForPmd(app);

}

ComPtr<ID3D12RootSignature> PmdRenderer::GetRootsignature()
{
	return rootsignature;
}

ComPtr<ID3D12PipelineState> PmdRenderer::GetPipelinestate()
{
	return pipelinestate;
}



void PmdRenderer::PreDrawPmd(DX12Application* pdx12)
{
	//PMD用の描画パイプラインに合わせる
	pdx12->_cmdList->SetPipelineState(pipelinestate.Get());
	////ルートシグネチャもPMD用に合わせる
	pdx12->_cmdList->SetGraphicsRootSignature(rootsignature.Get());
	pdx12->_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void PmdRenderer::PostDrawPmd(DX12Application* pdx12)
{
}

void PmdRenderer::PreDrawShadow(DX12Application* pdx12)
{
	auto commandList = pdx12->_cmdList;

	commandList->SetPipelineState(pdx12->shadowMapPls.Get());
	commandList->SetGraphicsRootSignature(rootsignature.Get());
	commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


HRESULT PmdRenderer::CreateGraphicsPipelineForPmd(DX12Application* app)
{
	ComPtr<ID3DBlob> vsBlob = nullptr;
	ComPtr<ID3DBlob> psBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	//頂点シェーダーオブジェクトの生成
	auto result = D3DCompileFromFile(
		L"BasicVertexShader.hlsl",  //シェーダー名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, //カレントディレクトリからインクルードする
		"BasicVS", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, //オプションにデバッグと最適化をスキップ
		0,
		&vsBlob, &errorBlob);

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
		&psBlob, &errorBlob);

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
	gpipeline.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength = vsBlob->GetBufferSize();
	gpipeline.PS.pShaderBytecode = psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = psBlob->GetBufferSize();
	//サンプルマスクとラスタライザーステートを設定
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//ブレンドステートの設定
	gpipeline.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);


	gpipeline.BlendState.RenderTarget[0].BlendEnable = true;
	gpipeline.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	gpipeline.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	gpipeline.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	gpipeline.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	gpipeline.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	gpipeline.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;

	//ラスタライザーの設定
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	//gpipeline.RasterizerState.MultisampleEnable = false;
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
	gpipeline.NumRenderTargets = 3;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpipeline.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpipeline.RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;

	//アンチエイリアシングのサンプル数の設定
	gpipeline.SampleDesc.Count = 1;
	gpipeline.SampleDesc.Quality = 0;

	//PSOの設定の終了

	//PSOの作成
	gpipeline.pRootSignature = rootsignature.Get();
	result = app->_dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(pipelinestate.ReleaseAndGetAddressOf()));


	//ShasowMap用シェーダー作成
	result = D3DCompileFromFile(
		L"BasicVertexShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"ShadowVS",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vsBlob,
		&errorBlob
	);

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

	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gpipeline.PS.BytecodeLength = 0;
	gpipeline.PS.pShaderBytecode = nullptr;
	gpipeline.NumRenderTargets = 0;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	gpipeline.RTVFormats[1] = DXGI_FORMAT_UNKNOWN;
	gpipeline.RTVFormats[2] = DXGI_FORMAT_UNKNOWN;


	result = app->_dev->CreateGraphicsPipelineState(
		&gpipeline,
		IID_PPV_ARGS(app->shadowMapPls.ReleaseAndGetAddressOf())
	);


	return result;
}

HRESULT PmdRenderer::CreateRootSignature(DX12Application* app)
{
	//ルートシグネチャに設定するルートパラメータ及びディスクリプタテーブル、ディスクリプタレンジの設定
	CD3DX12_DESCRIPTOR_RANGE descTblRange[5] = {};
	descTblRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);//CBV b0 viewProjectMatrix
	descTblRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);//CBV b1 worldTransformMatrix
	descTblRange[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);//CBV b2 Material
	descTblRange[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);//SRV t0 - t3 Textures
	descTblRange[4] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4); //SRV t4 ShadowMap

	//ルートパラメーターの設定
	CD3DX12_ROOT_PARAMETER rootparam[4] = {};

	rootparam[0].InitAsDescriptorTable(1, &descTblRange[0]);// view projection matrix;
	rootparam[1].InitAsDescriptorTable(1, &descTblRange[1]);// worldTransformMatrix;
	rootparam[2].InitAsDescriptorTable(2, &descTblRange[2]);// material;
	rootparam[3].InitAsDescriptorTable(1, &descTblRange[4]);// ShadowMap;


	//ルートシグネチャに設定するサンプラーの設定
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc[3] = {};

	samplerDesc[0].Init(0);
	samplerDesc[1].Init(1, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	//ソフトシャドウ用サンプラー
	samplerDesc[2] = samplerDesc[0];
	samplerDesc[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;   //<=で比較し、trueなら1.0、falseなら0.0を返す。
	samplerDesc[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; //比較結果をバイリニア補間
	samplerDesc[2].MaxAnisotropy = 1; // 深度傾斜を有効
	samplerDesc[2].ShaderRegister = 2;

	//ルートシグネチャの設定、生成
	//ルートシグネチャディスクリプタの設定
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//ルートパラメーターを設定
	rootSignatureDesc.pParameters = &rootparam[0];
	rootSignatureDesc.NumParameters = 4;
	//サンプラーを設定
	rootSignatureDesc.pStaticSamplers = samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 3;

	//ルートシグネチャのBlobの作成
	ComPtr<ID3DBlob> rootSigBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	auto result = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob,
		&errorBlob);

	//ルートシグネチャオブジェクトの作成
	result = app->_dev->CreateRootSignature(
		0,
		rootSigBlob->GetBufferPointer(),
		rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(rootsignature.ReleaseAndGetAddressOf()));



	return result;
}