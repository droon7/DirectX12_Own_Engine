#ifndef PMDACTOR_H
#define PMDACTOR_H

#include"pch.h"
#include"PmdData.h"
#include"DX12Application.h"
#include"PmdTexture.h"
#include"PmdBone.h"
#include"VMDData.h"
using Microsoft::WRL::ComPtr;

//16Byteアライメントのための構造体、ワールド座標、変換行列を入れる
struct Transform
{
	//new演算子をオーバーライドし、struct構造体メンバは16バイトで確保するようにする。
	void* operator new(size_t size);

	DirectX::XMMATRIX worldMatrix;
	std::vector<DirectX::XMMATRIX> boneMatrices;
};

//PMDモデル一キャラ分の情報を持つクラス
// PMDモデルの頂点、テクスチャ、マテリアルをロード、描画し、更新する。
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
	ComPtr<ID3D12Resource> whiteTex;
	ComPtr<ID3D12Resource> blackTex;
	ComPtr<ID3D12Resource> gradTex;

	//ワールド座標情報
	ComPtr<ID3D12DescriptorHeap> transformDescHeap; //バッファーの解釈する。実質ビュー
	ComPtr<ID3D12Resource> transformBuff; //実際のデータ
	Transform transform;
	DirectX::XMMATRIX* mapTransform = nullptr; //コピー用バッファ
	float angle;

	//ボーン情報
	PmdBone pmdBone ;
	//モーション情報
	//VMDData vmdData;
	//アニメーション情報
	DWORD startTime;


	//pmdモデルロード
	void LoadPmdData(std::string ModelName);
	//vbViewとibViewに設定
	void CreateVertexViewIndexView(DX12Application* app);
	//ワールド座標、座標変換行列をセット
	void SetTransform(int x, int y, int z);
	//座標変換行列情報をセット
	HRESULT CreateTransformView(DX12Application* app);
	//PMDデータからマテリアルのリソースを読み込む
	void GetMaterialResource(DX12Application* app);
	//pmdDataからテクスチャのリソースを読み込む
	void GetTextureResource(DX12Application* app);
	//materialの情報をもとにCBV、SRVを作成する
	void CreateMaterialAndTextureView(DX12Application* app);
	//ボーン情報アップデート
	void SetPmdBone(unsigned int frameNo);

	//デバッグ用関数 pmdBone::boneNodeTableをpublicにしないと動かない
	//void ShowIkBoneDebug();
public:

	explicit PmdActor(DX12Application* app, std::string ModelName, std::string motionPath, int x);

	//pmdモデル描画命令
	void DrawPmd(DX12Application* app);   

	void UpdatePmd(); //pmdモデルアップデート

	//アニメーション起動
	void PlayAnimation();

};

#endif