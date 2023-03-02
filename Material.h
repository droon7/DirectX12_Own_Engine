#ifndef MATERIAL_H
#define MATERIAL_H

#include"pch.h"
using Microsoft::WRL::ComPtr;

//マテリアル及び、テクスチャ、sph、spa、トゥーンテクスチャの情報を取り扱うクラス
class Material
{
public:
	Material();
	//マテリアル、テクスチャリソース情報
	std::vector<ComPtr<ID3D12Resource>> textureResources;
	std::vector<ComPtr<ID3D12Resource>> sphResources;
	std::vector<ComPtr<ID3D12Resource>> spaResources;
	std::vector<ComPtr<ID3D12Resource>> toonResources;

	//テクスチャをリソースにロードする
	ComPtr<ID3D12Resource> LoadTextureFromFile(std::string& texPath);


};

#endif 