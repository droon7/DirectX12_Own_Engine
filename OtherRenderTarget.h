#ifndef OTHERRENDERTARGET_H
#define  OTHERRENDERTARGET_H
#include"pch.h"
#include"DX12Application.h"
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
	std::array<ComPtr<ID3D12Resource>, 2> planeResources; //板ポリゴン用リソース 横方向ガウスブラーに使用
	ComPtr<ID3D12DescriptorHeap> planeRTVHeap; //板ポリ用RTV
	ComPtr<ID3D12DescriptorHeap> planeSRVHeap; //板ポリ用SRV、CBVも入る

	//以下は縦方向ガウスブラー用
	ComPtr<ID3D12Resource> planeResource2; //板ポリゴン用
	ComPtr<ID3D12PipelineState> planePipelinestate2 = nullptr; //


	ComPtr<ID3D12Resource> planePolygonVertexBuffer = {}; //板ポリバッファ
	D3D12_VERTEX_BUFFER_VIEW planePolygonVertexView = {}; //板ポリビュー
	planeVertex* mapPlaneVertex;

	ComPtr<ID3D12RootSignature> planeRootsignature = nullptr; //
	ComPtr<ID3D12PipelineState> planePipelinestate = nullptr; //

	//ボケパラメタリソース、CSV
	ComPtr<ID3D12Resource> bokehParameterBuffer;
	ComPtr<ID3D12DescriptorHeap> bokeCSVHeap;

	//法線マップ歪みテクスチャ用
	ComPtr<ID3D12DescriptorHeap> effectSRVHeap;
	ComPtr<ID3D12Resource> effectTextureBuffer; //めんどくさいのでPmdTexture使用

	//ブルーム用オブジェクト
	std::array<ComPtr<ID3D12Resource>, 2> bloomBuffer;
	ComPtr<ID3D12PipelineState> blurPipeline;

	//DOF用オブジェクト
	ComPtr<ID3D12Resource> dofBuffer;

	//別のRTV、ポストエフェクト用SRVの作成、
	void CreateRTVsAndSRVs(DX12Application* pdx12);
	//板ポリビューを作る
	void CreatePlanePolygon(DX12Application* pdx12);
	//ポストエフェクト用CSVの作成（現状ボケウェイトのみ）
	void CreateCBVForPostEffect(DX12Application* pdx12);
	//ポストエフェクト用ルートシグネチャ作成
	void CreateRootsignature(DX12Application* pdx12);
	//ポストエフェクト用PSO作成
	void CreateGraphicsPipeline(DX12Application* pdx12);
	//ポストエフェクト用テクスチャバッファ＋ビュー作成
	void CreateEffectBufferAndView(DX12Application* pdx12);
	//DOF用バッファー作成
	void CreateBlurForDOFBuffer(DX12Application* pdx12);


	//ガウス分布の確率分布関数からボケウェイトを得る
	std::vector<float> GetGaussianWeights(const size_t count, const float s);
public:
	OtherRenderTarget(DX12Application* pdx12);

	//planeResource1を描画。
	void DrawOtherRenderTarget(DX12Application* pdx12);

	//最初に描画するレンダーターゲットの前処理。現在はPMDモデルの描画に使用している
	void PreDrawOtherRenderTargets(DX12Application* pdx12);

	//最初に描画するレンダーターゲットの後処理。現在はPMDモデルの描画に使用している
	void PostDrawOtherRenderTargets(DX12Application* pdx12);

	//設定らも含めて描画する。現在
	void DrawOtherRenderTargetsFull(DX12Application* pdx12);

	//縮小バッファに対する書き込み
	void DrawShrinkTextureForBlur(DX12Application* pdx12);
};


#endif
