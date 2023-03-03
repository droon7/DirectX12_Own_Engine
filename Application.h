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

	//~Application() {
	//	debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
	//	debugDevice->Release();
	//};

	//�R�s�[�R���X�g���N�^�Ƒ�����Z�q���֎~
	DX12Application(const DX12Application&) = delete;
	void operator=(const DX12Application&) = delete;

public:
	int frame = 0;

	//Directx�p�C�v���C���I�u�W�F�N�g�̐錾
	D3D12_RECT scissorrect = {};
	D3D12_VIEWPORT viewport = {};
	ComPtr<IDXGIFactory6> _dxgiFactory ;
	ComPtr<IDXGISwapChain4> _swapchain ;
	ComPtr<ID3D12CommandAllocator> _cmdAllocator ;
	ComPtr<ID3D12GraphicsCommandList> _cmdList = nullptr;
	ComPtr<ID3D12CommandQueue> _cmdQueue = nullptr;
	ComPtr<ID3D12DescriptorHeap> rtvHeaps = nullptr;
	std::vector<ID3D12Resource*> _backBuffers;

	ComPtr<ID3D12RootSignature> rootsignature = nullptr;
	ComPtr<ID3D12PipelineState> _pipelinestate = nullptr;


	//�A�Z�b�g�̐錾
	ComPtr<ID3D12DescriptorHeap> matrixCsvHeaps = nullptr;

	//�����\��
	ComPtr<ID3D12Resource> vertBuff = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	ComPtr<ID3D12Resource> idxBuff = nullptr;
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	ComPtr<ID3D12Resource> sceneMatrixConstBuff = nullptr;

	ComPtr<ID3D12Resource> depthBuffer = nullptr;
	ComPtr<ID3D12DescriptorHeap> dsvHeaps = nullptr;

	//�e�N�X�`���A�Z�b�g
	D3D12_TEXTURE_COPY_LOCATION src = {};
	D3D12_TEXTURE_COPY_LOCATION dst = {};
	std::vector<ComPtr<ID3D12Resource>> textureResource;
	std::vector<ComPtr<ID3D12Resource>> sphResources;
	std::vector<ComPtr<ID3D12Resource>> spaResources;
	std::vector<ComPtr<ID3D12Resource>> toonResources;
	DirectX::TexMetadata metadata;
	DirectX::ScratchImage scratchImg;
	std::map<std::string, ComPtr<ID3D12Resource>> _resourceTable;
	ComPtr<ID3D12Resource> whiteTex;
	ComPtr<ID3D12Resource> blackTex;
	ComPtr<ID3D12Resource> gradTex;

	//�s��A�Z�b�g
	//DirectX::XMMATRIX matrix;
	DirectX::XMMATRIX worldMat;
	DirectX::XMMATRIX viewMat;
	DirectX::XMMATRIX projMat;
	SceneMatrix* mapMatrix = nullptr;
	float angle = 0.0f;

	//�}�e���A���f�[�^
	ComPtr<ID3D12Resource> materialBuff = nullptr;
	char* mapMaterial = nullptr;
	ComPtr<ID3D12DescriptorHeap> materialDescHeap = nullptr;

	//�V�F�[�_�[�I�u�W�F�N�g�̐錾
	ID3DBlob* _vsBlob = nullptr;
	ID3DBlob* _psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	//�����I�u�W�F�N�g�̐錾
	ComPtr<ID3D12Fence> _fence = nullptr;
	UINT64 _fenceVal = 0;
	HANDLE _fenceevent;

	//�f�o�b�O�I�u�W�F�N�g�̐錾
	ID3D12DebugDevice* debugDevice;


//WIC�e�N�X�`���̃��[�h
	ComPtr<ID3D12Resource> LoadTextureFromFile(std::string& texPath);

//���e�N�X�`���̍쐬
	ComPtr<ID3D12Resource> CreateWhiteTexture();
	ComPtr<ID3D12Resource> CreateBlackTexture();
	ComPtr<ID3D12Resource> CreateGradationTexture();

// PMD���[�_�[
	PmdData pmdData;



	ComPtr<ID3D12Device> _dev;

	static DX12Application* Instance(UINT width, UINT height);
	void OnInit();
	void OnUpdate();
	void OnRender();
	void OnDestroy();
	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandList();

	//�E�B���h�E�T�C�Y
	UINT window_width;
	UINT window_height;
	//�����_�[�^�[�Q�b�g�ƂȂ�o�b�t�@�̐�
	static const int buffer_count = 2;

	//�[�x�o�b�t�@�r���[�����
	HRESULT CreateDepthStencilView();
	//�r���[�s��A���ˍs�񂩂���V�[���̃r���[�����
	HRESULT CreateSceneView();
	//�`��J�n���\�b�h�A�����_�[�^�[�Q�b�g�A�o���A�A�[�x�r���[�A�r���[�|�[�g�̃R�}���h�ǉ�
	void BeginDraw();
	// �J�������Z�b�g���V�[����ݒ�
	void SetScene();
	//�`��I�����\�b�h�A�o���A�ݒ�A�R�}���h���X�g���s�A�t�F���X�ɂ�铯���A�R�}���h�̃��Z�b�g�A��ʂ̃X���b�v�ɂ��f�B�X�v���C�ւ̕`��
	void EndDraw();
	//GPU���R�}���h��S�Ď��s��������܂ő҂��ACPU�Ɠ�������
	void WaitForPreviousFrame();
};






//�[����؂�̂Ă郁�\�b�h
inline size_t AlignmentedSize(size_t size, size_t alignment)
{
	return size + alignment - size % alignment;
};

//���f���p�X����t�@�C��������菜���A�e�N�X�`���p�X�ƍ������郁�\�b�h
inline std::string GetTexturePathFromModelAndTexPath(const std::string& modelPath, const char* texPath)
{
	auto folderPath = modelPath.substr(0, modelPath.rfind('/')+1);
	return folderPath + texPath;
}

//�����񂩂烏�C�h������𓾂郁�\�b�h
std::wstring GetWideStringFromString(const std::string& str);

//�t�@�C��������g���q�𓾂郁�\�b�h
std::string GetExtension(const std::string& path);

//�e�N�X�`���̃p�X���Z�p���[�^�[�ŕ������郁�\�b�h
std::pair<std::string, std::string> SplitFileName(const std::string& path, const char splitter);

#endif