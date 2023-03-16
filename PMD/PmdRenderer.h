#ifndef PMDRENDERER_H
#define PMDRENDERER_H
#include"pch.h"
#include"DX12Application.h"
#include"PmdActor.h"

//PMDƒ‚ƒfƒ‹‚ğ•`‰æ‚·‚éÛ‚Ì‘S‘Ìİ’è‚ğ‚·‚é
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
	
	void EndDrawPmd(DX12Application* pdx12); //Œ»İ‚Í‹ó
};


#endif
