#ifndef PMDACTOR_H
#define PMDACTOR_H

#include"pch.h"
#include"PmdManager.h"
#include"Application.h"
#include"PmdTexture.h"
using Microsoft::WRL::ComPtr;

//PMDモデル一キャラ分の情報を持つクラス
// PMDモデルの頂点、テクスチャ、マテリアルをロード、更新する

class PmdActor
{
private:
	//PMD基礎データ
	std::string stringModelPath;
	PmdData pmdData;

	//頂点情報
	ComPtr<ID3D12Resource> vertBuff = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	ComPtr<ID3D12Resource> idxBuff = nullptr;
	D3D12_INDEX_BUFFER_VIEW ibView = {};

	//マテリアル、テクスチャ情報
	ComPtr<ID3D12DescriptorHeap> materialDescHeap;
	ComPtr<ID3D12Resource> materialBuff;
	PmdTexture pmdTexture;

	//ワールド座標情報
	DirectX::XMMATRIX worldMatrix;

public:

	//ComPtr<ID3D12Resource> LoadTextureFromFile(std::string& texPath);
	//pmdモデルロード
	void LoadPmdData(std::string ModelName);
	//vbViewとibViewに設定
	void CreateVertexViewIndexView(DX12Application* app); 
	//PMDデータからマテリアルのリソースを読み込む
	void GetMaterialResource(DX12Application* app);
	//pmdDataからテクスチャのリソースを読み込む
	void GetTextureResource(DX12Application* app);
	//materialの情報をもとにCBV、SRVを作成する
	void CreateMaterialAndTextureView(DX12Application* app);
	//座標変換行列情報をセット
	void CreateTransformView();

	//pmdモデル描画命令
	void PmdDraw(DX12Application* app);   

	void PmdUpdate(); //pmdモデルアップデート、現在は空

};

#endif