#include "pch.h"
#include "Shadow.h"



Shadow::Shadow(DX12Application* pdx12)
{
	CreateDepthMapObjects(pdx12);
}

//シャドウマップ用SRV作成
void Shadow::CreateDepthMapObjects(DX12Application* pdx12)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	auto result = pdx12->_dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&depthSRVHeap));

	D3D12_SHADER_RESOURCE_VIEW_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R32_FLOAT;
	resDesc.Texture2D.MipLevels = 1;
	resDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	resDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	auto handle = depthSRVHeap->GetCPUDescriptorHandleForHeapStart();
	pdx12->_dev->CreateShaderResourceView(pdx12->depthBuffer.Get(), &resDesc, handle);
}