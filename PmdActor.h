#ifndef PMDACTOR_H
#define PMDACTOR_H

#include"pch.h"
#include"PmdManager.h"
#include"Application.h"
#include"Material.h"
using Microsoft::WRL::ComPtr;

//PMDモデル一キャラ分の情報を持つクラス
// PMDモデルの頂点、テクスチャ、マテリアルをロード、更新する

class PmdActor
{
private:
	std::string stringModelPath;
	PmdData pmdData;

	//頂点情報
	ComPtr<ID3D12Resource> vertBuff = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	ComPtr<ID3D12Resource> idxBuff = nullptr;
	D3D12_INDEX_BUFFER_VIEW ibView = {};

	//マテリアル、テクスチャ情報
	MaterialData material;

	//ワールド座標情報
	DirectX::XMMATRIX worldMatrix;

public:

	//ComPtr<ID3D12Resource> LoadTextureFromFile(std::string& texPath);
	//pmdモデルロード
	void LoadPmdData(std::string ModelName);
	//vbViewとibViewに設定
	void CreateVertexViewIndexView(Application* app); 
	//textureResources等を設定
	void CreateMaterialAndTextureVIEW(Application* app);
	void PmdDraw(Application* app);   //pmdモデル描画命令

	void PmdUpdate(); //pmdモデルアップデート、現在は空

};

#endif