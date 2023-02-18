#ifndef TUTORIAL_H
#define TUTORIAL_H

#include <Windows.h>
#include <tchar.h>
#include<vector>
#include<string>
#include<wrl.h>
#ifdef _DEBUG
#include <iostream>
#endif

#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<d3dcompiler.h>
#include<DirectXTex.h>
#include<d3dx12.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTex.lib")


using Microsoft::WRL::ComPtr;

//PMD�w�b�_�[�\����
struct PMDHeader
{
	float version;
	char model_name[20];
	char comment[256];
};

class Dx12
{
public:
	Dx12(UINT window_width, UINT window_height);
	//~Dx12();

	void OnInit();
	void OnUpdate();
	void OnRender();
	void OnDestroy();
	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandList();
	void WaitForPreviousFrame();

	//�E�B���h�E�T�C�Y
	UINT window_width;
	UINT window_height;
	//�����_�[�^�[�Q�b�g�ƂȂ�o�b�t�@�̐�
	static const int buffer_count = 2;

private:
	int frame = 0;

	//Directx�p�C�v���C���I�u�W�F�N�g�̐錾
	D3D12_RECT scissorrect = {};
	D3D12_VIEWPORT viewport = {};
	ComPtr<ID3D12Device> _dev ;
	ComPtr<IDXGIFactory6> _dxgiFactory ;
	ComPtr<IDXGISwapChain4> _swapchain ;
	ComPtr<ID3D12CommandAllocator> _cmdAllocator ;
	ComPtr<ID3D12GraphicsCommandList> _cmdList = nullptr;
	ComPtr<ID3D12CommandQueue> _cmdQueue = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	ComPtr<ID3D12DescriptorHeap> rtvHeaps = nullptr;
	ComPtr<ID3D12DescriptorHeap> basicDescHeaps = nullptr;
	ComPtr<ID3D12RootSignature> rootsignature = nullptr;
	ComPtr<ID3D12PipelineState> _pipelinestate = nullptr;
	std::vector<ID3D12Resource*> _backBuffers;

	//�A�Z�b�g�̐錾

	ComPtr<ID3D12Resource> vertBuff = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	ComPtr<ID3D12Resource> idxBuff = nullptr;
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	D3D12_TEXTURE_COPY_LOCATION src = {};
	D3D12_TEXTURE_COPY_LOCATION dst = {};
	ComPtr<ID3D12Resource> texbuff = nullptr;
	ComPtr<ID3D12Resource> constBuff = nullptr;

	//�s��A�Z�b�g
	DirectX::XMMATRIX matrix;
	DirectX::XMMATRIX* mapMatrix = nullptr;
	DirectX::XMMATRIX worldMat;
	DirectX::XMMATRIX viewMat;
	DirectX::XMMATRIX projMat;
	float angle = 0.0f;


	//�V�F�[�_�[�I�u�W�F�N�g�̐錾
	ID3DBlob* _vsBlob = nullptr;
	ID3DBlob* _psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;


	//�����I�u�W�F�N�g�̐錾
	ComPtr<ID3D12Fence> _fence = nullptr;
	UINT64 _fenceVal = 0;
	HANDLE _fenceevent;

	//PMD�f�[�^�̐錾
	PMDHeader pmdheader;

	static constexpr size_t pmdvertex_size = 38;
	unsigned int vertNum;

};



//�E�B���h�E�A�v���N���X
class Win32App
{
public:
	static int WindowRun(Dx12* pdx12);
	static HWND GetHwnd() { return m_hwnd; } 

	static void DebugOutputFormatString(const char* format, ...); //�E�B���h�E�f�o�b�O�p�֐�
	LRESULT static WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam); //�E�B���h�E�������֐�

private:
	 static HWND m_hwnd;
};


//�[����؂�̂Ă郁�\�b�h
inline size_t AlignmentedSize(size_t size, size_t alignment)
{
	return size + alignment - size % alignment;
};



#endif