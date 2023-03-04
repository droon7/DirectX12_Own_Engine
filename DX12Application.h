#ifndef APPLICATIONL_H
#define APPLICATIONL_H

#include"pch.h"
#include"PmdManager.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTex.lib")


using Microsoft::WRL::ComPtr;

//�V�F�[�_�[�ɓn�����߂̊�{�I�ȍs��f�[�^
struct SceneMatrix
{
	//DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
	DirectX::XMFLOAT3 eye;
};


class DX12Application
{
private:

	//�V���O���g���N���X�ɂ��邽�߃R���X�g���N�^��private
	DX12Application(UINT window_width, UINT window_height);
	//ReportLiveDeviceObjects���Ă�
	//~Application() {
	//	debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
	//	debugDevice->Release();
	//};
	//�R�s�[�R���X�g���N�^�Ƒ�����Z�q���֎~
	DX12Application(const DX12Application&) = delete;
	void operator=(const DX12Application&) = delete;

	//�t���[����
	int frame = 0;
	//�E�B���h�E�T�C�Y
	UINT window_width;
	UINT window_height;
	//�����_�[�^�[�Q�b�g�ƂȂ�o�b�t�@�̐�
	static const int buffer_count = 2;

	//Directx�p�C�v���C���I�u�W�F�N�g�̐錾
	D3D12_RECT scissorrect = {};
	D3D12_VIEWPORT viewport = {};
	ComPtr<IDXGIFactory6> _dxgiFactory ;
	ComPtr<ID3D12DescriptorHeap> rtvHeaps = nullptr;
	std::vector<ID3D12Resource*> _backBuffers;

	//�A�Z�b�g�̐錾
	ComPtr<ID3D12DescriptorHeap> matrixCsvHeaps = nullptr;
	ComPtr<ID3D12Resource> sceneMatrixConstBuff = nullptr;

	ComPtr<ID3D12Resource> depthBuffer = nullptr;
	ComPtr<ID3D12DescriptorHeap> dsvHeaps = nullptr;

	//�s��A�Z�b�g
	DirectX::XMMATRIX viewMat;
	DirectX::XMMATRIX projMat;
	SceneMatrix* mapMatrix = nullptr;
	float angle = 0.0f;


	//�����I�u�W�F�N�g�̐錾


	//�f�o�b�O�I�u�W�F�N�g�̐錾
	ID3D12DebugDevice* debugDevice;


	void PopulateCommandList();
	HRESULT InitializeDXGIDevice();
	HRESULT InitializeCommands();
	HRESULT CreateSwapChain(const HWND& hwnd);
	HRESULT CreateFinalRenderTargets();
	//�[�x�o�b�t�@�r���[�����
	HRESULT CreateDepthStencilView();
	//�r���[�s��A���ˍs�񂩂���V�[���̃r���[�����
	HRESULT CreateSceneView();

public:
	ComPtr<ID3D12Device> _dev;
	ComPtr<IDXGISwapChain4> _swapchain;
	ComPtr<ID3D12GraphicsCommandList> _cmdList = nullptr;
	ComPtr<ID3D12CommandAllocator> _cmdAllocator;
	ComPtr<ID3D12CommandQueue> _cmdQueue = nullptr;
	ComPtr<ID3D12Fence> _fence = nullptr;
	UINT64 _fenceVal = 0;
	HANDLE _fenceevent;

	void OnInit(const HWND& hwnd);
	void OnUpdate();
	void OnRender();
	void OnDestroy();
	void LoadPipeline();
	void LoadAssets();
	//�V���O���g���p�^�[���̂���static�̐������\�b�h
	static DX12Application* Instance(UINT width, UINT height);
	//�`��J�n���\�b�h�A�����_�[�^�[�Q�b�g�A�o���A�A�[�x�r���[�A�r���[�|�[�g�̃R�}���h�ǉ�
	void BeginDraw();
	// �J�������Z�b�g���V�[����ݒ�
	void SetScene();
	//�`��I�����\�b�h�A�o���A�ݒ�A�R�}���h���X�g���s�A�t�F���X�ɂ�铯���A�R�}���h�̃��Z�b�g�A��ʂ̃X���b�v�ɂ��f�B�X�v���C�ւ̕`��
	void EndDraw();
	//GPU���R�}���h��S�Ď��s��������܂ő҂��ACPU�Ɠ�������
	void WaitForPreviousFrame();
};








#endif