#ifndef PMDACTOR_H
#define PMDACTOR_H

#include"pch.h"
#include"PmdManager.h"
#include"Application.h"
#include"PmdTexture.h"
using Microsoft::WRL::ComPtr;

//16Byteアライメントのための構造体、ワールド座標、変換行列（予定）を入れる
struct Transform
{
	//new演算子をオーバーライドし、struct構造体メンバは16バイトで確保するようにする。
	void* operator new(size_t size);

	DirectX::XMMATRIX worldMatrix;
};

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
	ComPtr<ID3D12DescriptorHeap> materialDescHeap; //バッファーの解釈する。実質ビュー
	ComPtr<ID3D12Resource> materialBuff; //実際のデータ
	PmdTexture pmdTexture; //データが置かれている

	//ワールド座標情報
	ComPtr<ID3D12DescriptorHeap> transformDescHeap; //バッファーの解釈する。実質ビュー
	ComPtr<ID3D12Resource> transformBuff; //実際のデータ
	Transform transform;
	float angle;

	//pmdモデルロード
	void LoadPmdData(std::string ModelName);
	//vbViewとibViewに設定
	void CreateVertexViewIndexView(DX12Application* app);
	//ワールド座標、座標変換行列をセット
	void SetTransform();
	//座標変換行列情報をセット
	void CreateTransformView(DX12Application* app);
	//PMDデータからマテリアルのリソースを読み込む
	void GetMaterialResource(DX12Application* app);
	//pmdDataからテクスチャのリソースを読み込む
	void GetTextureResource(DX12Application* app);
	//materialの情報をもとにCBV、SRVを作成する
	void CreateMaterialAndTextureView(DX12Application* app);

public:

	explicit PmdActor(DX12Application* app, std::string ModelName);

	//pmdモデル描画命令
	void DrawPmd(DX12Application* app);   

	void UpdatePmd(); //pmdモデルアップデート、現在は空

};

#endif