#ifndef PMDRENDERER_H
#define PMDRENDERER_H
#include"pch.h"
#include"DX12Application.h"
#include"PmdActor.h"

//PMDモデルを描画する際の全体設定をする
class PmdRenderer
{
private:
	ComPtr<ID3D12RootSignature> rootsignature = nullptr;
	ComPtr<ID3D12PipelineState> pipelinestate = nullptr;

	HRESULT CreateRootSignature(DX12Application* app);
	HRESULT CreateGraphicsPipelineForPmd(DX12Application* app);

public:
	explicit PmdRenderer(DX12Application* app);
	ComPtr<ID3D12RootSignature> GetRootsignature();
	ComPtr<ID3D12PipelineState> GetPipelinestate();
	
	void PreDrawPmd(DX12Application* pdx12);
	void PostDrawPmd(DX12Application* pdx12); //現在は空

	void PreDrawShadow(DX12Application* pdx12);
};


#endif
