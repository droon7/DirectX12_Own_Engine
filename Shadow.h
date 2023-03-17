#ifndef SHADOW_H
#define SHADOW_H

#include"pch.h"
#include"DX12Application.h"

using Microsoft::WRL::ComPtr;

class DX12Application;
class Shadow
{
private:
	//�[�x�}�b�v�p
	ComPtr<ID3D12DescriptorHeap> depthSRVHeap;
	//�V���h�E�}�b�v�p
	ComPtr<ID3D12Resource> lightDepthBuffer;
public:
	Shadow(DX12Application* pdx12);

	//�[�x�}�b�v�p�I�u�W�F�N�g�쐬�i���݂�SRV�̂݁j
	void CreateDepthMapObjects(DX12Application* pdx12);
};


#endif
