#ifndef PMDACTOR_H
#define PMDACTOR_H

#include"PmdManager.h"
#include"Application.h"

class PmdActor
{
private:
	std::string stringModelPath;
	PmdData pmdData;

	ComPtr<ID3D12Resource> vertBuff = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	ComPtr<ID3D12Resource> idxBuff = nullptr;
	D3D12_INDEX_BUFFER_VIEW ibView = {};

	DirectX::XMMATRIX worldMatrix;

public:

	//ComPtr<ID3D12Resource> LoadTextureFromFile(std::string& texPath);
	void PmdModelLoad(std::string odelName); //pmd���f�����[�h�@vbView��ibView�ɐݒ�

	void PmdUpdate(); //pmd���f���A�b�v�f�[�g�A���݂͋�H
	void PmdDraw();   //pmd���f���`�施��

};

#endif