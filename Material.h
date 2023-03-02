#ifndef MATERIAL_H
#define MATERIAL_H

#include"pch.h"
using Microsoft::WRL::ComPtr;

//�}�e���A���y�сA�e�N�X�`���Asph�Aspa�A�g�D�[���e�N�X�`���̏�����舵���N���X
class Material
{
public:
	Material();
	//�}�e���A���A�e�N�X�`�����\�[�X���
	std::vector<ComPtr<ID3D12Resource>> textureResources;
	std::vector<ComPtr<ID3D12Resource>> sphResources;
	std::vector<ComPtr<ID3D12Resource>> spaResources;
	std::vector<ComPtr<ID3D12Resource>> toonResources;

	//�e�N�X�`�������\�[�X�Ƀ��[�h����
	ComPtr<ID3D12Resource> LoadTextureFromFile(std::string& texPath);


};

#endif 