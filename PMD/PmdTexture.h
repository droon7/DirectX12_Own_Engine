#ifndef MATERIAL_H
#define MATERIAL_H

#include"pch.h"
#include"DX12Application.h"
using Microsoft::WRL::ComPtr;

//PMDモデルのマテリアル及び、テクスチャ、sph、spa、トゥーンテクスチャの情報を取り扱うクラス
class PmdTexture
{
public:
	PmdTexture();
	//マテリアル、テクスチャリソース情報
	std::vector<ComPtr<ID3D12Resource>> textureResources;
	std::vector<ComPtr<ID3D12Resource>> sphResources;
	std::vector<ComPtr<ID3D12Resource>> spaResources;
	std::vector<ComPtr<ID3D12Resource>> toonResources;

	//flyweightパターンのためのキャッシュ
	std::map<std::string, ComPtr<ID3D12Resource>> _resourceTable;

	//テクスチャをリソースにロードする
	ComPtr<ID3D12Resource> LoadTextureFromFile(std::string& texPath, DX12Application* app);

	//白、黒、グレーグラデーションテクスチャを作る
	ComPtr<ID3D12Resource> CreateWhiteTexture(DX12Application* app);
	ComPtr<ID3D12Resource> CreateBlackTexture(DX12Application* app);
	ComPtr<ID3D12Resource> CreateGradationTexture(DX12Application* app);


};

#endif 