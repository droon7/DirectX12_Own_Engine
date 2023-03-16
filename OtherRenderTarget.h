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
	ComPtr<ID3D12DescriptorHeap> planeRTVHeap; //板ポリ用RTV
	ComPtr<ID3D12DescriptorHeap> planeSRVHeap; //板ポリ用SRV

	ComPtr<ID3D12Resource> planePolygonVertexBuffer = {}; //板ポリバッファ
	D3D12_VERTEX_BUFFER_VIEW planePolygonVertexView = {}; //板ポリビュー
	planeVertex* mapPlaneVertex;

	ComPtr<ID3D12RootSignature> planeRootsignature = nullptr; //
	ComPtr<ID3D12PipelineState> planePipelinestate = nullptr; //



	void CreateRTVAndSRV(DX12Application* pdx12);

public:
	OtherRenderTarget(DX12Application* pdx12);

	//板ポリビューを作る
	void CreatePlanePolygon(DX12Application* pdx12);
	//ルートシグネチャを作る
	void CreateRootsignature(DX12Application* pdx12);
	//パイプラインステートを作る
	void CreateGraphicsPipeline(DX12Application* pdx12);

	void DrawOtherRenderTarget(DX12Application* pdx12);

	void PreDrawOtherRenderTargets(DX12Application* pdx12);

	void PostDrawOtherRenderTargets(DX12Application* pdx12);
};


#endif
