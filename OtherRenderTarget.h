#ifndef OTHERRENDERTARGET_H
#define  OTHERRENDERTARGET_H

#include"DX12Application.h"
#include"pch.h"
using Microsoft::WRL::ComPtr;

//板ポリ頂点
struct planeVertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 uv;
};


class DX12Application;

//ポストエフェクト等用の追加のレンダーターゲットクラス
class OtherRenderTarget
{
private:
	ComPtr<ID3D12Resource> planeResource; //板ポリゴン用リソース

	ComPtr<ID3D12DescriptorHeap> planeRTVHeap;
	ComPtr<ID3D12DescriptorHeap> planeSRVHeap;

	ComPtr<ID3D12Resource> planePolygonVertexBuffer = {}; //
	D3D12_VERTEX_BUFFER_VIEW planePolygonVertexView = {};
	planeVertex* mapPlaneVertex;

	void CreateRTVAndSRV(DX12Application* pdx12);

public:
	OtherRenderTarget() {}
	OtherRenderTarget(DX12Application* pdx12);

	//板ポリビューを作る
	void CreatePlanePolygon(DX12Application* pdx12);

	void DrawOtherRenderTarget(DX12Application* pdx12);
};


#endif
