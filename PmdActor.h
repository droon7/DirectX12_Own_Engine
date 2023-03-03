#ifndef PMDACTOR_H
#define PMDACTOR_H

#include"pch.h"
#include"PmdManager.h"
#include"Application.h"
#include"PmdTexture.h"
using Microsoft::WRL::ComPtr;

//16Byte�A���C�����g�̂��߂̍\���́A���[���h���W�A�ϊ��s��i�\��j������
struct Transform
{
	//new���Z�q���I�[�o�[���C�h���Astruct�\���̃����o��16�o�C�g�Ŋm�ۂ���悤�ɂ���B
	void* operator new(size_t size);

	DirectX::XMMATRIX worldMatrix;
};

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
	ComPtr<ID3D12DescriptorHeap> materialDescHeap; //�o�b�t�@�[�̉��߂���B�����r���[
	ComPtr<ID3D12Resource> materialBuff; //���ۂ̃f�[�^
	PmdTexture pmdTexture; //�f�[�^���u����Ă���

	//���[���h���W���
	ComPtr<ID3D12DescriptorHeap> transformDescHeap; //�o�b�t�@�[�̉��߂���B�����r���[
	ComPtr<ID3D12Resource> transformBuff; //���ۂ̃f�[�^
	Transform transform;
	float angle;

	//pmd���f�����[�h
	void LoadPmdData(std::string ModelName);
	//vbView��ibView�ɐݒ�
	void CreateVertexViewIndexView(DX12Application* app);
	//���[���h���W�A���W�ϊ��s����Z�b�g
	void SetTransform();
	//���W�ϊ��s������Z�b�g
	void CreateTransformView(DX12Application* app);
	//PMD�f�[�^����}�e���A���̃��\�[�X��ǂݍ���
	void GetMaterialResource(DX12Application* app);
	//pmdData����e�N�X�`���̃��\�[�X��ǂݍ���
	void GetTextureResource(DX12Application* app);
	//material�̏������Ƃ�CBV�ASRV���쐬����
	void CreateMaterialAndTextureView(DX12Application* app);

public:

	explicit PmdActor(DX12Application* app, std::string ModelName);

	//pmd���f���`�施��
	void DrawPmd(DX12Application* app);   

	void UpdatePmd(); //pmd���f���A�b�v�f�[�g�A���݂͋�

};

#endif