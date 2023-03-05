#ifndef MATERIAL_H
#define MATERIAL_H

#include"pch.h"
#include"DX12Application.h"
using Microsoft::WRL::ComPtr;

//PMD���f���̃}�e���A���y�сA�e�N�X�`���Asph�Aspa�A�g�D�[���e�N�X�`���̏�����舵���N���X
class PmdTexture
{
public:
	PmdTexture();
	//�}�e���A���A�e�N�X�`�����\�[�X���
	std::vector<ComPtr<ID3D12Resource>> textureResources;
	std::vector<ComPtr<ID3D12Resource>> sphResources;
	std::vector<ComPtr<ID3D12Resource>> spaResources;
	std::vector<ComPtr<ID3D12Resource>> toonResources;

	//flyweight�p�^�[���̂��߂̃L���b�V��
	std::map<std::string, ComPtr<ID3D12Resource>> _resourceTable;

	//�e�N�X�`�������\�[�X�Ƀ��[�h����
	ComPtr<ID3D12Resource> LoadTextureFromFile(std::string& texPath, DX12Application* app);

	//���A���A�O���[�O���f�[�V�����e�N�X�`�������
	ComPtr<ID3D12Resource> CreateWhiteTexture(DX12Application* app);
	ComPtr<ID3D12Resource> CreateBlackTexture(DX12Application* app);
	ComPtr<ID3D12Resource> CreateGradationTexture(DX12Application* app);


};

#endif 