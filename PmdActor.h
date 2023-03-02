#ifndef PMDACTOR_H
#define PMDACTOR_H

#include"pch.h"
#include"PmdManager.h"
#include"Application.h"
#include"PmdTexture.h"
using Microsoft::WRL::ComPtr;

//PMD���f����L�������̏������N���X
// PMD���f���̒��_�A�e�N�X�`���A�}�e���A�������[�h�A�X�V����

class PmdActor
{
private:
	//PMD��b�f�[�^
	std::string stringModelPath;
	PmdData pmdData;

	//���_���
	ComPtr<ID3D12Resource> vertBuff = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	ComPtr<ID3D12Resource> idxBuff = nullptr;
	D3D12_INDEX_BUFFER_VIEW ibView = {};

	//�}�e���A���A�e�N�X�`�����
	PmdTexture texture;

	//���[���h���W���
	DirectX::XMMATRIX worldMatrix;

public:

	//ComPtr<ID3D12Resource> LoadTextureFromFile(std::string& texPath);
	//pmd���f�����[�h
	void LoadPmdData(std::string ModelName);
	//vbView��ibView�ɐݒ�
	void CreateVertexViewIndexView(DX12Application* app); 
	//material��pmdData�̊e�����i�[����B
	void GetResourceMaterialAndTexture(DX12Application* app);
	//material�̏������Ƃ�CBV�ASRV���쐬����
	void CreateMaterialAndTextureView(DX12Application* app);

	//pmd���f���`�施��
	void PmdDraw(DX12Application* app);   

	void PmdUpdate(); //pmd���f���A�b�v�f�[�g�A���݂͋�

};

#endif