#ifndef OTHERRENDERTARGET_H
#define  OTHERRENDERTARGET_H

#include"DX12Application.h"
#include"pch.h"
using Microsoft::WRL::ComPtr;

//�|�����_
struct planeVertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 uv;
};


class DX12Application;

//�|�X�g�G�t�F�N�g���p�̒ǉ��̃����_�[�^�[�Q�b�g�N���X
class OtherRenderTarget
{
private:
	ComPtr<ID3D12Resource> planeResource; //�|���S���p���\�[�X
	ComPtr<ID3D12DescriptorHeap> planeRTVHeap; //�|���pRTV
	ComPtr<ID3D12DescriptorHeap> planeSRVHeap; //�|���pSRV�ACBV������

	ComPtr<ID3D12Resource> planePolygonVertexBuffer = {}; //�|���o�b�t�@
	D3D12_VERTEX_BUFFER_VIEW planePolygonVertexView = {}; //�|���r���[
	planeVertex* mapPlaneVertex;

	ComPtr<ID3D12RootSignature> planeRootsignature = nullptr; //
	ComPtr<ID3D12PipelineState> planePipelinestate = nullptr; //

	//�{�P�p�����^���\�[�X�ACSV
	ComPtr<ID3D12Resource> bokehParameterBuffer;
	float* mappedWeight;


	//�ʂ�RTV�A�|�X�g�G�t�F�N�g�pSRV�̍쐬
	void CreateRTVAndSRV(DX12Application* pdx12);
	//�|���r���[�����
	void CreatePlanePolygon(DX12Application* pdx12);
	//�|�X�g�G�t�F�N�g�pCSV�̍쐬�i����{�P�E�F�C�g�p�j
	void CreateCBVForPostEffect(DX12Application* pdx12);
	//�|�X�g�G�t�F�N�g�p���[�g�V�O�l�`���쐬
	void CreateRootsignature(DX12Application* pdx12);
	//�|�X�g�G�t�F�N�g�pPSO�쐬
	void CreateGraphicsPipeline(DX12Application* pdx12);

	//�K�E�X���z�̊m�����z�֐�����{�P�E�F�C�g�𓾂�
	std::vector<float> GetGaussianWeights(const size_t count, const float s);
public:
	OtherRenderTarget(DX12Application* pdx12);

	//planeResource��`��
	void DrawOtherRenderTarget(DX12Application* pdx12);

	//�ŏ��ɕ`�悷�郌���_�[�^�[�Q�b�g�̑O�����B���݂�PMD���f���̕`��Ɏg�p���Ă���
	void PreDrawOtherRenderTargets(DX12Application* pdx12);

	//�ŏ��ɕ`�悷�郌���_�[�^�[�Q�b�g�̌㏈���B���݂�PMD���f���̕`��Ɏg�p���Ă���
	void PostDrawOtherRenderTargets(DX12Application* pdx12);
};


#endif
