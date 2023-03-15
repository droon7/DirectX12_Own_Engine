#include "pch.h"
#include "OtherRenderTarget.h"

OtherRenderTarget::OtherRenderTarget(DX12Application* pdx12)
{
	CreateRTVAndSRV(pdx12);

	CreatePlanePolygon(pdx12);

	CreateRootsignature(pdx12);
	CreateGraphicsPipeline(pdx12);
}


//RTV、RTVヒープ、SRVヒープを作る
//
void OtherRenderTarget::CreateRTVAndSRV(DX12Application* pdx12)
{
	//RTVリソース作成
	auto rtvResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, pdx12->window_width, pdx12->window_height);
	rtvResourceDesc.Alignment = 65536;
	rtvResourceDesc.MipLevels = 1;
	rtvResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	float clsColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	D3D12_CLEAR_VALUE clearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, clsColor);

	auto result = pdx12->_dev->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&rtvResourceDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearValue,
		IID_PPV_ARGS(planeResource.ReleaseAndGetAddressOf())
	);

	//RTV作成
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 1;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	result = pdx12->_dev->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(planeRTVHeap.ReleaseAndGetAddressOf())
	);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	pdx12->_dev->CreateRenderTargetView(
		planeResource.Get(),
		&rtvDesc,
		planeRTVHeap->GetCPUDescriptorHandleForHeapStart());

	//SRV作成
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	result = pdx12->_dev->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(planeSRVHeap.ReleaseAndGetAddressOf())
	);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = rtvDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	pdx12->_dev->CreateShaderResourceView(
		planeResource.Get(),
		&srvDesc,
		planeSRVHeap->GetCPUDescriptorHandleForHeapStart()
	);
}

//板ポリバッファー、ビュー作成
void OtherRenderTarget::CreatePlanePolygon(DX12Application* pdx12)
{
	planeVertex pv[4] = { {{-1,-1,0.1}, {0,1}},
						  {{-1, 1,0.1}, {0,0}},
						  {{ 1,-1,0.1}, {1,1}},
						  {{ 1 ,1,0.1}, {1,0}}  };

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(pv));

	auto result = pdx12->_dev->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(planePolygonVertexBuffer.ReleaseAndGetAddressOf())
	);

	planePolygonVertexView.BufferLocation = planePolygonVertexBuffer->GetGPUVirtualAddress();
	planePolygonVertexView.SizeInBytes = sizeof(pv);
	planePolygonVertexView.StrideInBytes = sizeof(planeVertex);

	planePolygonVertexBuffer->Map(0, nullptr, (void**)&mapPlaneVertex);
	std::copy(std::begin(pv), std::end(pv), mapPlaneVertex);
	planePolygonVertexBuffer->Unmap(0, nullptr);
}

void OtherRenderTarget::CreateRootsignature(DX12Application* pdx12)
{
	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.NumParameters = 0;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	
	ComPtr<ID3DBlob> rsBlob;
	ComPtr<ID3DBlob> errBlob;

	auto result = D3D12SerializeRootSignature(
		&rsDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		rsBlob.ReleaseAndGetAddressOf(),
		errBlob.ReleaseAndGetAddressOf()
	);

	if (FAILED(result)) {
		assert(0);
		return;
	}

	result = pdx12->_dev->CreateRootSignature(
		0,
		rsBlob->GetBufferPointer(),
		rsBlob->GetBufferSize(),
		IID_PPV_ARGS(planeRootsignature.ReleaseAndGetAddressOf())
	);

	if (FAILED(result)) {
		assert(0);
		return;
	}
}

void OtherRenderTarget::CreateGraphicsPipeline(DX12Application* pdx12)
{
	//シェーダーへの入力レイアウト作成
	D3D12_INPUT_ELEMENT_DESC layout[2] =
	{
		{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};
	gpsDesc.InputLayout.NumElements = _countof(layout);
	gpsDesc.InputLayout.pInputElementDescs = layout;

	ComPtr<ID3DBlob> planeVs; //板ポリシェーダー
	ComPtr<ID3DBlob> planePs; //
	ComPtr<ID3DBlob> errorBlob; //

	auto result = D3DCompileFromFile(
		L"planeVertex.hlsl", nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"vs", "vs_5_0", 0, 0,
		planeVs.ReleaseAndGetAddressOf(),
		errorBlob.ReleaseAndGetAddressOf()
	);

	if (FAILED(result)) {
		assert(0);
		return;
	}

	result = D3DCompileFromFile(
		L"planePixel.hlsl", nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"ps", "ps_5_0", 0, 0,
		planePs.ReleaseAndGetAddressOf(),
		errorBlob.ReleaseAndGetAddressOf()
	);

	if (FAILED(result)) {
		assert(0);
		return;
	}

	gpsDesc.VS = CD3DX12_SHADER_BYTECODE(planeVs.Get());
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(planePs.Get());

	gpsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsDesc.NumRenderTargets = 1;
	gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpsDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpsDesc.SampleDesc.Count = 1;
	gpsDesc.SampleDesc.Quality = 0;
	gpsDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	gpsDesc.pRootSignature = planeRootsignature.Get();

	result = pdx12->_dev->CreateGraphicsPipelineState(
		&gpsDesc,
		IID_PPV_ARGS(planePipelinestate.ReleaseAndGetAddressOf())
	);

	if (FAILED(result)) {
		assert(0);
		return;
	}
}


//planeResourceを描画
void OtherRenderTarget::DrawOtherRenderTarget(DX12Application* pdx12)
{
	auto rtvHeapPointer = planeRTVHeap->GetCPUDescriptorHandleForHeapStart();

	auto BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
		planeResource.Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);

	pdx12->_cmdList->ResourceBarrier(1, &BarrierDesc);

	auto dsvHead = pdx12->dsvHeaps->GetCPUDescriptorHandleForHeapStart();
	pdx12->_cmdList->OMSetRenderTargets(
		1, &rtvHeapPointer, false,
		&dsvHead
	);

	BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
		planeResource.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);

	pdx12->_cmdList->ResourceBarrier(1, &BarrierDesc);
}
