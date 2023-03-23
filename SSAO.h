#ifndef SSAO_H
#define SSAO_H


using Microsoft::WRL::ComPtr;

//SSAO��`�悷�邽�߂̃N���X
class SSAO
{
private :
	//SSAO�p�I�u�W�F�N�g
	ComPtr<ID3D12Resource> ssaoBuffer;
	ComPtr<ID3D12PipelineState> ssaoPipeline;
	ComPtr<ID3D12DescriptorHeap> ssaoRTVHeap;
	ComPtr<ID3D12DescriptorHeap> ssaoSRVHeap;


public :
	SSAO();
	SSAO(ComPtr<ID3D12Device> device);

	//ssao�p�o�b�t�@�[�쐬
	HRESULT CreateSSAOBuffer(ComPtr<ID3D12Device> device);

	HRESULT CreateSSAODescriptorHeap(ComPtr<ID3D12Device> device);


};


#endif
