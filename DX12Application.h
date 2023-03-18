#ifndef APPLICATIONL_H
#define APPLICATIONL_H

#include"pch.h"
#include"PmdData.h"
#include"OtherRenderTarget.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTex.lib")
#pragma comment(lib, "Winmm.lib")


using Microsoft::WRL::ComPtr;

//�V�F�[�_�[�ɓn�����߂̊�{�I�ȍs��f�[�^
struct SceneMatrix
{
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
	DirectX::XMMATRIX lightCamera;  //�V���h�E�}�b�v�p
	DirectX::XMMATRIX shadow;		//�e�s��p
	DirectX::XMFLOAT3 eye;
};

class OtherRenderTarget;
class PmdRenderer;
//DirectX12�̑S�̂̐ݒ������N���X�B
class DX12Application
{
	friend OtherRenderTarget;
	friend PmdRenderer;
private:

	//�V���O���g���N���X�ɂ��邽�߃R���X�g���N�^��private
	DX12Application(UINT window_width, UINT window_height);

	//ReportLiveDeviceObjects���Ă�
	//~Application() {
	//	debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
	//	debugDevice->Release();
	//};
	// 
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
	std::vector<ComPtr<ID3D12Resource>> _backBuffers;

	//�A�Z�b�g�̐錾
	ComPtr<ID3D12DescriptorHeap> matrixCsvHeaps = nullptr;
	ComPtr<ID3D12Resource> sceneMatrixConstBuff = nullptr;

	ComPtr<ID3D12Resource> depthBuffer = nullptr;
	ComPtr<ID3D12DescriptorHeap> dsvHeaps = nullptr;



	//�s��A�Z�b�g
	DirectX::XMMATRIX viewMat;
	DirectX::XMMATRIX projMat;
	SceneMatrix* mapTransform = nullptr;
	float angle = 0.0f;

	//�e�p�A�Z�b�g
	static constexpr unsigned int shadowDifinition = 2048;
	DirectX::XMFLOAT3 parallelLightVec;
	DirectX::XMMATRIX lightMat;
	ComPtr<ID3D12Resource> shadowMapBuffer = nullptr; 
	ComPtr<ID3D12DescriptorHeap> depthSRVHeaps = nullptr;
	ComPtr<ID3D12PipelineState> shadowMapPls = nullptr;

	//�f�o�b�O�I�u�W�F�N�g�̐錾
	ID3D12DebugDevice* debugDevice;

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
	//void OnUpdate();
	//void OnRender();
	void OnDestroy();
	void LoadPipeline();
	//void LoadAssets();
	// 
	//�V���O���g���p�^�[���̂���static�̐������\�b�h
	static DX12Application* Instance(UINT width, UINT height);
	//�o�b�N�o�b�t�@�̕`��J�n���\�b�h�A�����_�[�^�[�Q�b�g�A�o���A�A�[�x�r���[�A�r���[�|�[�g�̃R�}���h�ǉ�
	void SetBackBufferToRTV();
	//�o�b�N�o�b�t�@�̃��\�[�X�o���A�̐ݒ�
	void EndBackBufferDraw();
	// �J�������Z�b�g���V�[����ݒ�
	void SetScene();
	// �V���h�E�}�b�v��`�悷�邽�߂̑O�ݒ������B
	void PreDrawShadowMap();
	//�`��I�����\�b�h�A�o���A�ݒ�A�R�}���h���X�g���s�A�t�F���X�ɂ�铯���A�R�}���h�̃��Z�b�g�A��ʂ̃X���b�v�ɂ��f�B�X�v���C�ւ̕`��
	void EndDraw();
	//GPU���R�}���h��S�Ď��s��������܂ő҂��ACPU�Ɠ�������
	void WaitForPreviousFrame();

	//�s�N�`�����[�h
	void LoadPictureFromFile(std::wstring filepath, ComPtr<ID3D12Resource>& buff);
};








#endif