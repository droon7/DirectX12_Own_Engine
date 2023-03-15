#ifndef OTHERRENDERTARGET_H
#define  OTHERRENDERTARGET_H

#include"DX12Application.h"
#include"pch.h"
using Microsoft::WRL::ComPtr;

class DX12Application;

class OtherRenderTarget
{
private:
	ComPtr<ID3D12Resource> planeResource; //板ポリゴン用リソース

	ComPtr<ID3D12DescriptorHeap> planeRTVHeap;
	ComPtr<ID3D12DescriptorHeap> planeSRVHeap;

public:
	OtherRenderTarget() {}
	OtherRenderTarget(DX12Application* pdx12);

	void CreateRTVAndSRV(DX12Application* pdx12);

	void DrawOtherRenderTarget(DX12Application* pdx12);
};


#endif
