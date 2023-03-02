#ifndef PMDACTOR_H
#define PMDACTOR_H

#include"PmdManager.h"
#include"Application.h"
//PMD���f����L�������̏������N���X
// PMD���f���̒��_�A�e�N�X�`���A�}�e���A�������[�h�A�X�V����

class PmdActor
{
private:
	std::string stringModelPath;
	PmdData pmdData;

	//���_���
	ComPtr<ID3D12Resource> vertBuff = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	ComPtr<ID3D12Resource> idxBuff = nullptr;
	D3D12_INDEX_BUFFER_VIEW ibView = {};

	//�}�e���A���A�e�N�X�`�����
	std::vector<ComPtr<ID3D12Resource>> textureResource;
	std::vector<ComPtr<ID3D12Resource>> sphResources;
	std::vector<ComPtr<ID3D12Resource>> spaResources;
	std::vector<ComPtr<ID3D12Resource>> toonResources;

	//���[���h���W���
	DirectX::XMMATRIX worldMatrix;

public:

	//ComPtr<ID3D12Resource> LoadTextureFromFile(std::string& texPath);
	void LoadPmdData(std::string ModelName);
	void CreateVertexViewIndexView(Application* app); //pmd���f�����[�h�@vbView��ibView�ɐݒ�
	void PmdDraw(Application* app);   //pmd���f���`�施��

	void PmdUpdate(); //pmd���f���A�b�v�f�[�g�A���݂͋�

};

#endif