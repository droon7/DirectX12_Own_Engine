#ifndef PMDACTOR_H
#define PMDACTOR_H

#include"PmdManager.h"
#include"Application.h"
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
	std::vector<ComPtr<ID3D12Resource>> textureResource;
	std::vector<ComPtr<ID3D12Resource>> sphResources;
	std::vector<ComPtr<ID3D12Resource>> spaResources;
	std::vector<ComPtr<ID3D12Resource>> toonResources;

	//ワールド座標情報
	DirectX::XMMATRIX worldMatrix;

public:

	//ComPtr<ID3D12Resource> LoadTextureFromFile(std::string& texPath);
	void LoadPmdData(std::string ModelName);
	void CreateVertexViewIndexView(Application* app); //pmdモデルロード　vbViewとibViewに設定
	void PmdDraw(Application* app);   //pmdモデル描画命令

	void PmdUpdate(); //pmdモデルアップデート、現在は空

};

#endif