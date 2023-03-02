#ifndef PMDRENDERER_H
#define PMDRENDERER_H
#include"pch.h"
#include"Application.h"
#include"PmdActor.h"

class PmdRenderer
{
private:
	ComPtr<ID3D12RootSignature> rootsignature = nullptr;
	ComPtr<ID3D12PipelineState> pipelinestate = nullptr;


	HRESULT CreateGraphicsPipelineForPmd(DX12Application* app);
	HRESULT CreateRootSignature(DX12Application* app);

public:
};


#endif
