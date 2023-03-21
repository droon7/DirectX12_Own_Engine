#include "pch.h"
#include "OtherRenderTarget.h"
#include "Utility.h"


OtherRenderTarget::OtherRenderTarget(DX12Application* pdx12)
{
	CreateRTVsAndSRVs(pdx12);
	CreateCBVForPostEffect(pdx12);
	CreateEffectBufferAndView(pdx12);

	CreatePlanePolygon(pdx12);

	CreateRootsignature(pdx12);
	CreateGraphicsPipeline(pdx12);
}


//RTV、RTVヒープ、SRVヒープを作る
void OtherRenderTarget::CreateRTVsAndSRVs(DX12Application* pdx12)
{
	//RTVリソース作成、２つ作る
	auto rtvResourceDesc = pdx12->_backBuffers[0]->GetDesc();

	D3D12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	float clsColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	D3D12_CLEAR_VALUE clearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, clsColor);
	for (auto& resource : planeResources) {
		auto result = pdx12->_dev->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&rtvResourceDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&clearValue,
			IID_PPV_ARGS(resource.ReleaseAndGetAddressOf())
		);
		if (FAILED(result)) {
			assert(0);
			return;
		}
	}



	auto result = pdx12->_dev->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&rtvResourceDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearValue,
		IID_PPV_ARGS(planeResource2.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) {
		assert(0);
		return;
	}

	//RTV作成、２つ作る
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 3;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	result = pdx12->_dev->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(planeRTVHeap.ReleaseAndGetAddressOf())
	);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	auto handle = planeRTVHeap->GetCPUDescriptorHandleForHeapStart();

	for (auto& resource : planeResources)
	{
		pdx12->_dev->CreateRenderTargetView(
			resource.Get(),
			&rtvDesc,
			handle);
		handle.ptr += pdx12->_dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}


	pdx12->_dev->CreateRenderTargetView(
		planeResource2.Get(),
		&rtvDesc,
		handle);


	//SRV作成
	heapDesc.NumDescriptors = 3;
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

	handle = planeSRVHeap->GetCPUDescriptorHandleForHeapStart();

	for (auto& resource : planeResources)
	{
		pdx12->_dev->CreateShaderResourceView(
			resource.Get(),
			&srvDesc,
			handle
		);
		handle.ptr += pdx12->_dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}


	pdx12->_dev->CreateShaderResourceView(
		planeResource2.Get(),
		&srvDesc,
		handle
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

	planePolygonVertexBuffer->Map(0, nullptr, (void**)&mapPlaneVertex);
	std::copy(std::begin(pv), std::end(pv), mapPlaneVertex);
	planePolygonVertexBuffer->Unmap(0, nullptr);

	planePolygonVertexView.BufferLocation = planePolygonVertexBuffer->GetGPUVirtualAddress();
	planePolygonVertexView.SizeInBytes = sizeof(pv);
	planePolygonVertexView.StrideInBytes = sizeof(planeVertex);

}

//ボケウェイトでCSVを作る
void OtherRenderTarget::CreateCBVForPostEffect(DX12Application* pdx12)
{
	std::vector<float> gaussWeights = GetGaussianWeights(8, 5.0f);

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(static_cast<int>(AlignmentedSize(sizeof(gaussWeights[0]) * gaussWeights.size(), 256)));

	auto result = pdx12->_dev->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(bokehParameterBuffer.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) {
		assert(0);
		return;
	}
	float* mappedWeight = nullptr;
	result = bokehParameterBuffer->Map(0, nullptr, (void**)&mappedWeight);
	if (FAILED(result)) {
		assert(0);
		return;
	}
	copy(gaussWeights.begin(), gaussWeights.end(), mappedWeight);
	bokehParameterBuffer->Unmap(0, nullptr);

	auto handle = planeSRVHeap->GetCPUDescriptorHandleForHeapStart();
	//間にSRVがもう一つ入るため2*incrementSizeにする
	handle.ptr += 2 * pdx12->_dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = bokehParameterBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = bokehParameterBuffer->GetDesc().Width;
	pdx12->_dev->CreateConstantBufferView(&cbvDesc, handle);
}

//ポストエフェクト用ルートシグネチャ作成
void OtherRenderTarget::CreateRootsignature(DX12Application* pdx12)
{
	D3D12_DESCRIPTOR_RANGE range[4] = {};
	range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[0].BaseShaderRegister = 0;
	range[0].NumDescriptors = 1;
	range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	range[1].BaseShaderRegister = 0;
	range[1].NumDescriptors = 1;
	range[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[2].BaseShaderRegister = 1;
	range[2].NumDescriptors = 1;
	range[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[3].BaseShaderRegister = 2;
	range[3].NumDescriptors = 1;


	D3D12_ROOT_PARAMETER rp[4] = {};
	rp[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // RTV
	rp[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rp[0].DescriptorTable.pDescriptorRanges = range;
	rp[0].DescriptorTable.NumDescriptorRanges = 1;
	rp[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //gauss weight cbv
	rp[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rp[1].DescriptorTable.pDescriptorRanges = &range[1];
	rp[1].DescriptorTable.NumDescriptorRanges = 1;
	rp[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //normal map
	rp[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rp[2].DescriptorTable.pDescriptorRanges = &range[2];
	rp[2].DescriptorTable.NumDescriptorRanges = 1;
	rp[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //depth map
	rp[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rp[3].DescriptorTable.pDescriptorRanges = &range[3];
	rp[3].DescriptorTable.NumDescriptorRanges = 1;

	D3D12_STATIC_SAMPLER_DESC sampler = CD3DX12_STATIC_SAMPLER_DESC(0);

	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.NumParameters = 4;
	rsDesc.pParameters = rp;
	rsDesc.NumStaticSamplers = 1;
	rsDesc.pStaticSamplers = &sampler;
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

//ポストエフェクト用PSO作成
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
		}
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
	gpsDesc.DepthStencilState.DepthEnable = false;
	gpsDesc.DepthStencilState.StencilEnable = false;
	gpsDesc.NumRenderTargets = 2;
	gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsDesc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;

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

	result = D3DCompileFromFile(
		L"planePixel.hlsl", nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"VerticalBokePS", "ps_5_0", 0, 0,
		planePs.ReleaseAndGetAddressOf(),
		errorBlob.ReleaseAndGetAddressOf()
	);

	if (FAILED(result)) {
		assert(0);
		return;
	}

	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(planePs.Get());

	result = pdx12->_dev->CreateGraphicsPipelineState(
		&gpsDesc,
		IID_PPV_ARGS(planePipelinestate2.ReleaseAndGetAddressOf())
	);

	if (FAILED(result)) {
		assert(0);
		return;
	}

}

//ポストエフェクト用SRV作成
void OtherRenderTarget::CreateEffectBufferAndView(DX12Application* pdx12)
{
	pdx12->LoadPictureFromFile(L"normal/normalmap.jpg", effectTextureBuffer);

	//ポストエフェクト用ディスクリプタヒープ作成
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	auto result = pdx12->_dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(effectSRVHeap.ReleaseAndGetAddressOf()));

	if (FAILED(result)) {
		assert(0);
		return;
	}

	auto desc = effectTextureBuffer->GetDesc();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = desc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	pdx12->_dev->CreateShaderResourceView(
		effectTextureBuffer.Get(),
		&srvDesc,
		effectSRVHeap->GetCPUDescriptorHandleForHeapStart()
	);
}



std::vector<float> OtherRenderTarget::GetGaussianWeights(const size_t count, const float s)
{
	std::vector<float> weights(count);
	float x = 0.0f;
	float total = 0.0f;
	//ガウス分布における離散化された確率密度関数を計算する。
	//離散化のためcountの段階分e^(-x^2 / -2s)を計算し、その総和が1となるように調整する
	for (auto& wgt : weights)
	{
		wgt = expf(-(x * x) / (2 * s * s)); 
		total += wgt;
		x += 1.0f;
	}
	total = total * 2.0f - 1; //実際にガウス分布は正負両方のため２倍にし、0の分は重複しかつe^0=1のため1は引く

	//weightsの総和が1となるよう総和で割る
	for (auto& wgt : weights)
	{
		wgt /= total;
	}

	return weights;
}


//planeResourceを描画
void OtherRenderTarget::DrawOtherRenderTarget(DX12Application* pdx12)
{
	pdx12->_cmdList->SetGraphicsRootSignature(planeRootsignature.Get());
	pdx12->_cmdList->SetPipelineState(planePipelinestate.Get());
	pdx12->_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	pdx12->_cmdList->IASetVertexBuffers(0, 1, &planePolygonVertexView);

	pdx12->_cmdList->SetDescriptorHeaps(1, planeSRVHeap.GetAddressOf());
	auto handle = planeSRVHeap->GetGPUDescriptorHandleForHeapStart();
	handle.ptr += pdx12->_dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	pdx12->_cmdList->SetGraphicsRootDescriptorTable(0, handle);

	handle.ptr += pdx12->_dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	pdx12->_cmdList->SetDescriptorHeaps(1, pdx12->depthSRVHeaps.GetAddressOf());
	pdx12->_cmdList->SetGraphicsRootDescriptorTable(3, pdx12->depthSRVHeaps->GetGPUDescriptorHandleForHeapStart());
	pdx12->_cmdList->SetDescriptorHeaps(1, effectSRVHeap.GetAddressOf());
	pdx12->_cmdList->SetGraphicsRootDescriptorTable(2, effectSRVHeap->GetGPUDescriptorHandleForHeapStart());

	pdx12->_cmdList->SetDescriptorHeaps(1, pdx12->depthSRVHeaps.GetAddressOf());
	pdx12->_cmdList->SetGraphicsRootDescriptorTable(3, pdx12->depthSRVHeaps->GetGPUDescriptorHandleForHeapStart());

	pdx12->_cmdList->DrawInstanced(4,1,0,0);
}

//最初に描画するレンダーターゲットの前処理。現在はPMDモデルの描画に使用している
//このクラスが保持するRTVを設定している。
void OtherRenderTarget::PreDrawOtherRenderTargets(DX12Application* pdx12)
{
	for (auto& resource : planeResources)
	{
		auto BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
			resource.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		);
		pdx12->_cmdList->ResourceBarrier(1, &BarrierDesc);
	}





	auto rtvHeapPointer = planeRTVHeap->GetCPUDescriptorHandleForHeapStart();
	auto dsvHead = pdx12->dsvHeaps->GetCPUDescriptorHandleForHeapStart();
	pdx12->_cmdList->OMSetRenderTargets(
		1, &rtvHeapPointer, false, &dsvHead);
	float clsClr[4] = { 0.5,0.5,0.5,1.0 };
	pdx12->_cmdList->ClearRenderTargetView(rtvHeapPointer, clsClr, 0, nullptr);
	pdx12->_cmdList->ClearDepthStencilView(dsvHead, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	D3D12_VIEWPORT vp = CD3DX12_VIEWPORT(0.0f, 0.0f, pdx12->window_width, pdx12->window_height);
	pdx12->_cmdList->RSSetViewports(1, &vp);//ビューポート

	CD3DX12_RECT rc(0, 0, pdx12->window_width, pdx12->window_height);
	pdx12->_cmdList->RSSetScissorRects(1, &rc);//シザー(切り抜き)矩形



	//シャドウマップ描画のためのディスクリプタヒープ設定
	//シャドウマップDSVが指してるリソースをSRVとして設定
	pdx12->_cmdList->SetDescriptorHeaps(1, pdx12->depthSRVHeaps.GetAddressOf());
	auto handle = pdx12->depthSRVHeaps->GetGPUDescriptorHandleForHeapStart();
	handle.ptr += pdx12->_dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	pdx12->_cmdList->SetGraphicsRootDescriptorTable(3, handle);
}

//最初に描画するレンダーターゲットの後処理。現在はPMDモデルの描画に使用している
void OtherRenderTarget::PostDrawOtherRenderTargets(DX12Application* pdx12)
{
		for (auto& resource : planeResources)
		{
			auto BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
				resource.Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
			);
			pdx12->_cmdList->ResourceBarrier(1, &BarrierDesc);
		}
}

//リソースバリアの設定、レンダーターゲットの設定も書く
void OtherRenderTarget::DrawOtherRenderTargetsFull(DX12Application* pdx12)
{
	auto BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
		planeResource2.Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	pdx12->_cmdList->ResourceBarrier(1, &BarrierDesc);

	//RTV関係命令
	auto rtvHeapPointer = planeRTVHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHeapPointer.ptr += pdx12->_dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	auto dsvHead = pdx12->dsvHeaps->GetCPUDescriptorHandleForHeapStart();
	pdx12->_cmdList->OMSetRenderTargets(
		1, &rtvHeapPointer, false, &dsvHead);
	float clsClr[4] = { 0.5,0.5,0.5,1.0 };
	pdx12->_cmdList->ClearRenderTargetView(rtvHeapPointer, clsClr, 0, nullptr);
	//pdx12->_cmdList->ClearDepthStencilView(dsvHead, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	D3D12_VIEWPORT vp = CD3DX12_VIEWPORT(0.0f, 0.0f, pdx12->window_width, pdx12->window_height);
	pdx12->_cmdList->RSSetViewports(1, &vp);//ビューポート

	CD3DX12_RECT rc(0, 0, pdx12->window_width, pdx12->window_height);
	pdx12->_cmdList->RSSetScissorRects(1, &rc);//シザー(切り抜き)矩形

	
	//描画関係命令
	pdx12->_cmdList->SetGraphicsRootSignature(planeRootsignature.Get());
	pdx12->_cmdList->SetPipelineState(planePipelinestate2.Get());
	pdx12->_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	pdx12->_cmdList->IASetVertexBuffers(0, 1, &planePolygonVertexView);

	pdx12->_cmdList->SetDescriptorHeaps(1, planeSRVHeap.GetAddressOf());
	auto handle = planeSRVHeap->GetGPUDescriptorHandleForHeapStart();
	//handle.ptr += pdx12->_dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	pdx12->_cmdList->SetGraphicsRootDescriptorTable(0, handle);

	handle.ptr += 2*pdx12->_dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	pdx12->_cmdList->SetGraphicsRootDescriptorTable(1, handle);




	pdx12->_cmdList->DrawInstanced(4, 1, 0, 0);


	BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
		planeResource2.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);

	pdx12->_cmdList->ResourceBarrier(1, &BarrierDesc);
}