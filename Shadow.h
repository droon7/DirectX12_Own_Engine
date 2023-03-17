#ifndef SHADOW_H
#define SHADOW_H

#include"pch.h"
#include"DX12Application.h"

using Microsoft::WRL::ComPtr;

class DX12Application;
class Shadow
{
private:
	//深度マップ用
	ComPtr<ID3D12DescriptorHeap> depthSRVHeap;
	//シャドウマップ用
	ComPtr<ID3D12Resource> lightDepthBuffer;
public:
	Shadow(DX12Application* pdx12);

	//深度マップ用オブジェクト作成（現在はSRVのみ）
	void CreateDepthMapObjects(DX12Application* pdx12);
};


#endif
