#ifndef SSAO_H
#define SSAO_H


using Microsoft::WRL::ComPtr;

//SSAOを描画するためのクラス
class SSAO
{
private :
	//SSAO用オブジェクト
	ComPtr<ID3D12Resource> ssaoBuffer;
	ComPtr<ID3D12PipelineState> ssaoPipeline;
	ComPtr<ID3D12DescriptorHeap> ssaoRTVHeap;
	ComPtr<ID3D12DescriptorHeap> ssaoSRVHeap;


public :
	SSAO();
	SSAO(ComPtr<ID3D12Device> device);

	//ssao用バッファー作成
	HRESULT CreateSSAOBuffer(ComPtr<ID3D12Device> device);

	HRESULT CreateSSAODescriptorHeap(ComPtr<ID3D12Device> device);


};


#endif
