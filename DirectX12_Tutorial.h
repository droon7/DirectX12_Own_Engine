#pragma once

#include <Windows.h>
#include <tchar.h>
#include<vector>
#include<string>
#ifdef _DEBUG
#include <iostream>
#endif

#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<d3dcompiler.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")





class Dx12
{
public:
	Dx12(UINT window_width, UINT window_height);
	//~Dx12();

	void OnInit();
	void OnUpdate();
	void OnRender();
	void OnDestroy();

	//ウィンドウサイズ
	UINT window_width;
	UINT window_height;

	static const int buffer_count = 2;



private:
	int frame = 0;

	//Directxパイプラインオブジェクトの宣言
	D3D12_RECT scissorrect = {};
	D3D12_VIEWPORT viewport = {};
	ID3D12Device* _dev = nullptr;
	IDXGIFactory6* _dxgiFactory = nullptr;
	IDXGISwapChain4* _swapchain = nullptr;
	ID3D12CommandAllocator* _cmdAllocator = nullptr;
	ID3D12GraphicsCommandList* _cmdList = nullptr;
	ID3D12CommandQueue* _cmdQueue = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	ID3D12DescriptorHeap* rtvHeaps = nullptr;
	ID3D12RootSignature* rootsignature = nullptr;
	ID3D12PipelineState* _pipelinestate = nullptr;
	std::vector<ID3D12Resource*> _backBuffers;

	//アセットの宣言

	ID3D12Resource* vertBuff = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	ID3D12Resource* idxBuff = nullptr;
	D3D12_INDEX_BUFFER_VIEW ibView = {};

	//同期オブジェクトの宣言
	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceVal = 0;
	HANDLE _fenceevent;

	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandList();
	void WaitForPreviousFrame();

};

//ウィンドウアプリクラス
class Win32App
{
public:
	static int WindowRun(Dx12* pdx12);
	static HWND GetHwnd() { return m_hwnd; } 

	static void DebugOutputFormatString(const char* format, ...); //ウィンドウデバッグ用関数
	LRESULT static WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam); //ウィンドウ初期化関数

private:
	 static HWND m_hwnd;
};

