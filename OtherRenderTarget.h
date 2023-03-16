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
	ComPtr<ID3D12DescriptorHeap> planeSRVHeap; //�|���pSRV

	ComPtr<ID3D12Resource> planePolygonVertexBuffer = {}; //�|���o�b�t�@
	D3D12_VERTEX_BUFFER_VIEW planePolygonVertexView = {}; //�|���r���[
	planeVertex* mapPlaneVertex;

	ComPtr<ID3D12RootSignature> planeRootsignature = nullptr; //
	ComPtr<ID3D12PipelineState> planePipelinestate = nullptr; //



	void CreateRTVAndSRV(DX12Application* pdx12);

public:
	OtherRenderTarget(DX12Application* pdx12);

	//�|���r���[�����
	void CreatePlanePolygon(DX12Application* pdx12);
	//���[�g�V�O�l�`�������
	void CreateRootsignature(DX12Application* pdx12);
	//�p�C�v���C���X�e�[�g�����
	void CreateGraphicsPipeline(DX12Application* pdx12);

	void DrawOtherRenderTarget(DX12Application* pdx12);

	void PreDrawOtherRenderTargets(DX12Application* pdx12);

	void PostDrawOtherRenderTargets(DX12Application* pdx12);
};


#endif
