#include "pch.h"
#include "OtherRenderTarget.h"

OtherRenderTarget::OtherRenderTarget(DX12Application* pdx12)
{
	CreateRTVAndSRV(pdx12);
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
