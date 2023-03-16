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
	ComPtr<ID3D12DescriptorHeap> planeSRVHeap; //板ポリ用SRV、CBVも入る

	ComPtr<ID3D12Resource> planePolygonVertexBuffer = {}; //板ポリバッファ
	D3D12_VERTEX_BUFFER_VIEW planePolygonVertexView = {}; //板ポリビュー
	planeVertex* mapPlaneVertex;

	ComPtr<ID3D12RootSignature> planeRootsignature = nullptr; //
	ComPtr<ID3D12PipelineState> planePipelinestate = nullptr; //

	//ボケパラメタリソース、CSV
	ComPtr<ID3D12Resource> bokehParameterBuffer;
	float* mappedWeight;


	//別のRTV、ポストエフェクト用SRVの作成
	void CreateRTVAndSRV(DX12Application* pdx12);
	//板ポリビューを作る
	void CreatePlanePolygon(DX12Application* pdx12);
	//ポストエフェクト用CSVの作成（現状ボケウェイト用）
	void CreateCBVForPostEffect(DX12Application* pdx12);
	//ポストエフェクト用ルートシグネチャ作成
	void CreateRootsignature(DX12Application* pdx12);
	//ポストエフェクト用PSO作成
	void CreateGraphicsPipeline(DX12Application* pdx12);

	//ガウス分布の確率分布関数からボケウェイトを得る
	std::vector<float> GetGaussianWeights(const size_t count, const float s);
public:
	OtherRenderTarget(DX12Application* pdx12);

	//planeResourceを描画
	void DrawOtherRenderTarget(DX12Application* pdx12);

	//最初に描画するレンダーターゲットの前処理。現在はPMDモデルの描画に使用している
	void PreDrawOtherRenderTargets(DX12Application* pdx12);

	//最初に描画するレンダーターゲットの後処理。現在はPMDモデルの描画に使用している
	void PostDrawOtherRenderTargets(DX12Application* pdx12);
};


#endif
