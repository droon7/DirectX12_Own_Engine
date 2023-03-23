#include "pch.h"
#include "SSAO.h"

SSAO::SSAO()
{
}

SSAO::SSAO(ComPtr<ID3D12Device> device)
{
	CreateSSAOBuffer(device);
	CreateSSAODescriptorHeap(device);
}

HRESULT SSAO::CreateSSAOBuffer(ComPtr<ID3D12Device> device)
{
	D3D12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32_FLOAT, 1980, 1080); //XXX:マジックナンバー
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Alignment = 65536;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	float clsColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	D3D12_CLEAR_VALUE clearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R32_FLOAT, clsColor);

	auto result = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearValue,
		IID_PPV_ARGS(ssaoBuffer.ReleaseAndGetAddressOf())
	);

	if (FAILED(result)) {
		assert(0);
		return false;
	}
	return true;
}

HRESULT SSAO::CreateSSAODescriptorHeap(ComPtr<ID3D12Device> device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 0;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	auto result = device->CreateDescriptorHeap(
		&desc,
		IID_PPV_ARGS(ssaoRTVHeap.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) {
		assert(0);
		return false;
	}

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	device->CreateRenderTargetView(
		ssaoBuffer.Get(), &rtvDesc, ssaoRTVHeap->GetCPUDescriptorHandleForHeapStart()
	);

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 0;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	result = device->CreateDescriptorHeap(
		&desc,
		IID_PPV_ARGS(ssaoSRVHeap.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) {
		assert(0);
		return false;
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	device->CreateShaderResourceView(
		ssaoBuffer.Get(), &srvDesc,
		ssaoSRVHeap->GetCPUDescriptorHandleForHeapStart()
	);

	return true;

}
