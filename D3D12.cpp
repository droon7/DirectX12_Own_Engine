#include "DirectX12_Tutorial.h"

using namespace::DirectX;

//�E�B���h�E�T�C�Y�̏����l������B�����_�[�^�[�Q�b�g��������B
Dx12::Dx12(UINT width, UINT height) :
	window_width(width),
	window_height(height),
	_backBuffers(buffer_count)
{
}

//�������A�p�C�v���C���̏������ƃA�Z�b�g�ނ̃��[�h�𕪂���

void Dx12::OnInit()
{
	LoadPipeline();
	LoadAssets();
}


//�p�C�v���C���ɕK�v�ȃI�u�W�F�N�g�̐����A���������s��
//TODO: �I�u�W�F�N�g�̏�������ThrowIfFailed()�֐��ɓ����
void Dx12::LoadPipeline()
{

#ifdef _DEBUG
	//�f�o�b�O���C���[�̗L����������

	ID3D12Debug* debugLayer = nullptr;
	auto result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));

	debugLayer->EnableDebugLayer(); //�f�o�b�O���C���[�̗L����
	debugLayer->Release(); //�C���^�[�t�F�C�X�̉��
	
#endif


	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	//Directx3D�f�o�C�X����
	//�񋓌^levels�𑖍����쐬�\�ȃo�[�W������DirectX12�̃f�o�C�X�����
	D3D_FEATURE_LEVEL featurelevel;
	for (auto lv : levels)
	{
		if (D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			featurelevel = lv;
			break;
		}
	}
	if (_dev == nullptr) {
		return;
	}

	//DXGIFactory����
#ifdef _DEBUG
	result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory));
#else
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
#endif

	//�R�}���h�A���P�[�^�[�̐���
	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&_cmdAllocator));

	//�R�}���h���X�g�̐���
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		_cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&_cmdList));

	//�R�}���h�L���[�̐ݒ肨��ѐ���
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; //�^�C���A�E�g�Ȃ�
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL; //�v���C�I���e�B�w��Ȃ�
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; //�R�}���h���X�g�Ɠ����^�C�v
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));

	//�X���b�v�`�F�[���̐ݒ肨��ѐ���
	swapchainDesc.Width = window_width;
	swapchainDesc.Height = window_height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = buffer_count;
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ComPtr<IDXGISwapChain1> swapChain;
	result = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue.Get(),
		Win32App::GetHwnd(),
		&swapchainDesc,
		nullptr,
		nullptr,
		&swapChain);
	swapChain.As(&_swapchain);

	//RTV�̃f�B�X�N���v�^�q�[�v�̐ݒ�
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = buffer_count;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));

	//�@SRGB�����_�[�^�[�Q�b�g�r���[
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	//�X���b�v�`�F�[���ƃf�B�X�N���v�^��R�Â������_�[�^�[�Q�b�g�r���[�𐶐�����B
	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = _swapchain->GetDesc(&swcDesc);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();

	for (int idx = 0; idx < swcDesc.BufferCount; ++idx) {
		result = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));  //_backBuffer�ɃX���b�v�`�F�[����̃o�b�N�o�b�t�@�̃�����������
		_dev->CreateRenderTargetView(_backBuffers[idx], &rtvDesc, handle);       //�o�b�t�@�̐���������
		handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);  //�|�C���^�������_�[�r���[�̑傫�������炷
	}

	//�t�F���X�̐���
	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));


}


//�A�Z�b�g�̃��[�h�A����͒��_�A���_�C���f�b�N�X�A�V�F�[�_�[�APSO�A���[�g�V�O�l�`����
void Dx12::LoadAssets()
{
	//���\�[�X�̐���
	// 
	//���_�f�[�^�\����
	struct Vertex
	{
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};

	////���_������ł�
	//Vertex vertices[] = {
	//	{{ -1.0f, -1.0f,0.0f} ,{0.0f, 1.0f}},//����
	//	{{ -1.0f,  1.0f,0.0f} ,{0.0f, 0.0f}},//����
	//	{{  1.0f, -1.0f,0.0f} ,{1.0f, 1.0f}},//�E��
	//	{{  1.0f,  1.0f,0.0f} ,{1.0f, 0.0f}},//�E��
	//};

	////���_�C���f�b�N�X������ł�
	//unsigned short indices[] = {
	//	0, 1, 2,
	//	2, 1, 3
	//};

	/*//�e�N�X�`��������ł�
	struct TexRGBA
	{
		unsigned char R, G, B, A;
	};

	std::vector<TexRGBA> texturedata(256 * 256);

	for (auto& rgba : texturedata)
	{
		rgba.R = rand() % 256;
		rgba.G = rand() % 256;
		rgba.B = rand() % 256;
		rgba.A = 255;
	}
	*/

	//PMD�w�b�_�[�ǂݍ���
	char signature[3] = {};
	FILE* fp ;
	auto err = fopen_s(&fp,"Model/�����~�N.pmd", "rb");

	fread(signature, sizeof(signature), 1, fp);
	fread(&pmdheader, sizeof(pmdheader), 1,fp);

	//PMD���_���ǂݍ���

	fread(&vertNum, sizeof(vertNum), 1, fp);
	
	std::vector<unsigned char> vertices(vertNum* pmdvertex_size);
	fread(vertices.data(), vertices.size(), 1, fp);

	//PMD�C���f�b�N�X�f�[�^�ǂݍ���
	std::vector<unsigned short> indices; //2�o�C�g�̃f�[�^����������unsigned short���g��
	fread(&indicesNum, sizeof(indicesNum), 1, fp);
	indices.resize(indicesNum);
	fread(indices.data(), indices.size() * sizeof(indices[0]), 1, fp);

	fclose(fp);



	//DirectXTex���C�u�����̃��\�b�h�ɂ��e�N�X�`���摜�����[�h
	auto result = CoInitializeEx(0, COINIT_MULTITHREADED);
	TexMetadata metadata = {};
	ScratchImage scratchImg = {};

	result = LoadFromWICFile(
		L"img/Family_Computer.png", WIC_FLAGS_NONE,
		&metadata, scratchImg);

	auto img = scratchImg.GetImage(0, 0, 0);


	//���_�o�b�t�@�[�̐���
	D3D12_HEAP_PROPERTIES heapprop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC resourcedesc = CD3DX12_RESOURCE_DESC::Buffer(vertices.size());

	vertBuff = nullptr;
	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resourcedesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff)
	);

	//Map���\�b�h�Ńr�f�I������vertBuff��ɒ��_����������
	unsigned char* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap); //vertMap���vertBuff�̉��z��������u��
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertBuff->Unmap(0, nullptr);  //���z������������

	//���_�o�b�t�@�[�r���[�̐���
	vbView = {};
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = vertices.size();
	vbView.StrideInBytes = pmdvertex_size;

	//�C���f�b�N�X�o�b�t�@�[�̐���
	idxBuff = nullptr;

	resourcedesc = CD3DX12_RESOURCE_DESC::Buffer(indices.size() * sizeof(indices[0]));

	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resourcedesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&idxBuff)
	);

	unsigned short* mappedIdx = nullptr;
	idxBuff->Map(0, nullptr, (void**)&mappedIdx);

	std::copy(std::begin(indices), std::end(indices), mappedIdx);
	idxBuff->Unmap(0, nullptr);

	//�C���f�b�N�X�o�b�t�@�[�r���[�̍쐬
	ibView = {};

	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = indices.size() * sizeof(indices[0]);



	//���_�V�F�[�_�[�I�u�W�F�N�g�̐���
	result = D3DCompileFromFile(
		L"BasicVertexShader.hlsl",  //�V�F�[�_�[��
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, //�J�����g�f�B���N�g������C���N���[�h����
		"BasicVS", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, //�I�v�V�����Ƀf�o�b�O�ƍœK�����X�L�b�v
		0,
		&_vsBlob, &errorBlob);

	//�G���[�������̏���
	if (FAILED(result)) {
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("�t�@�C������������܂���");
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			::OutputDebugStringA(errstr.c_str());
		}
		exit(1);
	}

	//�s�N�Z���V�F�[�_�[�I�u�W�F�N�g�̐���
	result = D3DCompileFromFile(
		L"BasicPixelShader.hlsl",  //�V�F�[�_�[��
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, //�J�����g�f�B���N�g������C���N���[�h����
		"BasicPS", "ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&_psBlob, &errorBlob);

	//�G���[�������̏���
	if (FAILED(result)) {
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("�t�@�C������������܂���");
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		exit(1);
	}

	//���_���C�A�E�g�̐ݒ�
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{//���W���
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{//�@���x�N�g�����
			"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{//uv���
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
			0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{//�{�[�����
			"BONE_NO", 0, DXGI_FORMAT_R16G16_UINT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{//�{�[���E�F�C�g���
			"WEIGHT", 0, DXGI_FORMAT_R8_UINT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{//�֊s���t���O���
			"EDGE_FLG", 0, DXGI_FORMAT_R8_UINT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};


	//�e�N�X�`���̒��ԃo�b�t�@�[�Ƃ��ẴA�b�v���[�h�q�[�v�̐ݒ�
	D3D12_HEAP_PROPERTIES uploadHeapProp = {};
	uploadHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	//UPLOAD�p�Ȃ̂�UNKNOWN�ɐݒ�
	uploadHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	uploadHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	uploadHeapProp.CreationNodeMask = 0;
	uploadHeapProp.VisibleNodeMask = 0;

	//�e�N�X�`���̒��ԃo�b�t�@���\�[�X�f�B�X�N���v�^�̐ݒ�
	//���ԃo�b�t�@�[�Ȃ̂Ńe�N�X�`���Ƃ��Ďw�肵�Ȃ�
	D3D12_RESOURCE_DESC textureBuffDesc = {};

	textureBuffDesc.Format = DXGI_FORMAT_UNKNOWN;
	textureBuffDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	textureBuffDesc.Width = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)
		* img->height;
	textureBuffDesc.Height = 1;
	textureBuffDesc.DepthOrArraySize = 1;
	textureBuffDesc.MipLevels = 1;
	textureBuffDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	textureBuffDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	textureBuffDesc.SampleDesc.Count = 1;
	textureBuffDesc.SampleDesc.Quality = 0;

	//�e�N�X�`���̒��ԃo�b�t�@�[�̍쐬
	ID3D12Resource* uploadbuff = nullptr;

	result = _dev->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&textureBuffDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadbuff)
	);

	//���ԃo�b�t�@����̃R�s�[��̃e�N�X�`���o�b�t�@�[�̍쐬

	D3D12_HEAP_PROPERTIES textureHeapProp = {};

	//�e�N�X�`���̃q�[�v�̐ݒ�
	textureHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	textureHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	textureHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	textureHeapProp.CreationNodeMask = 0;
	textureHeapProp.VisibleNodeMask = 0;

	//�e�N�X�`���̃��\�[�X�̐ݒ�
	textureBuffDesc.Format = metadata.format;
	textureBuffDesc.Width = metadata.width;
	textureBuffDesc.Height = metadata.height;
	textureBuffDesc.DepthOrArraySize = metadata.arraySize;
	textureBuffDesc.MipLevels = metadata.mipLevels;
	textureBuffDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
	textureBuffDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;


	//�R�s�[��̃e�N�X�`���o�b�t�@�[�̍쐬
	result = _dev->CreateCommittedResource(
		&textureHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&textureBuffDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&texbuff)
	);

	//���ԃo�b�t�@�[�փe�N�X�`���f�[�^���R�s�[
	uint8_t* mapforImg = nullptr;
	result = uploadbuff->Map(0, nullptr, (void**)&mapforImg);

	//std::copy_n(img->pixels, img->slicePitch, mapforImg);
	auto srcAddress = img->pixels;
	auto rowPitch = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

	for (int y = 0; y < img->height; ++y)
	{
		std::copy_n(srcAddress, rowPitch, mapforImg);

		srcAddress += img->rowPitch;
		mapforImg += rowPitch;
	}
	uploadbuff->Unmap(0, nullptr);


	//CommandList::CopyTextureRegion()�̈����̍\���̂����
	//�A�b�v���[�h���̍쐬
	src.pResource = uploadbuff;
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint.Offset = 0;
	src.PlacedFootprint.Footprint.Width = metadata.width;
	src.PlacedFootprint.Footprint.Height = metadata.height;
	src.PlacedFootprint.Footprint.Depth = metadata.depth;
	src.PlacedFootprint.Footprint.RowPitch = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	src.PlacedFootprint.Footprint.Format = img->format;

	//�R�s�[��̐ݒ�
	dst.pResource = texbuff.Get();
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;

	_cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	{

		//�e�N�X�`���̃R�s�[�̂��߂̃��\�[�X�o���A�̐ݒ�

		D3D12_RESOURCE_BARRIER BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
			texbuff.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
		);
		_cmdList->ResourceBarrier(1, &BarrierDesc);

		_cmdList->Close();

		ID3D12CommandList* cmdlists[] = { _cmdList.Get()};
		_cmdQueue->ExecuteCommandLists(1, cmdlists);

		_cmdQueue->Signal(_fence.Get(), ++_fenceVal);

		if (_fence->GetCompletedValue() != _fenceVal)
		{
			auto event = CreateEvent(nullptr, false, false, nullptr);
			_fence->SetEventOnCompletion(_fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}
		_cmdAllocator->Reset();
		_cmdList->Reset(_cmdAllocator.Get(), nullptr);
	}

	//�V�F�[�_�[���\�[�X�r���[�ƒ萔�o�b�t�@�[�r���[�p�̃f�B�X�N���v�^�q�[�v�̍쐬
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	//�V�F�[�_�[���猩����悤��
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = 2;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	result = _dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&basicDescHeaps));

	//�V�F�[�_�[���\�[�X�r���[�̍쐬
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	auto basicHeaphandle = basicDescHeaps->GetCPUDescriptorHandleForHeapStart();

	_dev->CreateShaderResourceView(
		texbuff.Get(),
		&srvDesc,
		basicHeaphandle
	);


	//�s��̓]���B�s�N�Z�����W�n����V�F�[�_�[���W�n�ւ̕ϊ��s��𑗂�B
	//XMMATRIX matrix = XMMatrixIdentity();
	//matrix.r[0].m128_f32[0] = 2.0f / window_width;
	//matrix.r[1].m128_f32[1] = -2.0f / window_height;
	//matrix.r[3].m128_f32[0] = -1.0f;
	//matrix.r[3].m128_f32[1] = 1.0f;

	//���[���h�s��A�r���[�s��A�v���W�F�N�V�����s����v�Z����Z���Ă���
	worldMat = XMMatrixRotationY(0);
	matrix = worldMat;

	XMFLOAT3 eye(0, 10, -15); 
	XMFLOAT3 target(0, 10, 0); // eye���W��target���W���王���x�N�g�������
	XMFLOAT3 up(0, 1, 0);

	viewMat = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
	matrix *= viewMat;

	projMat = XMMatrixPerspectiveFovLH(
		XM_PIDIV2,
		static_cast<float>(window_width) / static_cast<float>(window_height),
		1.0f,
		100.0f
	);
	matrix *= projMat;


	//�萔�o�b�t�@�[�̍쐬
	auto constHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto constHeapDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(matrix) + 0xff) & ~0xff);

	_dev->CreateCommittedResource(
		&constHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&constHeapDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuff)
	);
	
	//�}�b�v�ɂ��萔�̓]��
	result = constBuff->Map(0, nullptr, (void**)&mapMatrix);
	*mapMatrix = matrix;
	
	//�萔�o�b�t�@�[�r���[�̍쐬�̂��߂̐ݒ�
	basicHeaphandle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_CONSTANT_BUFFER_VIEW_DESC constBufferViewDesc = {};
	constBufferViewDesc.BufferLocation = constBuff->GetGPUVirtualAddress();
	constBufferViewDesc.SizeInBytes = constBuff->GetDesc().Width;

	//�萔�o�b�t�@�[�r���[�̍쐬
	_dev->CreateConstantBufferView(
		&constBufferViewDesc,
		basicHeaphandle
	);


	//�[�x�o�b�t�@�[�̍쐬

	//�[�x�o�b�t�@�[�f�B�X�N���v�^�̐ݒ�
	D3D12_RESOURCE_DESC depthResourceDescriptor = {};
	depthResourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResourceDescriptor.Width = window_width;
	depthResourceDescriptor.Height = window_height;
	depthResourceDescriptor.DepthOrArraySize = 1;
	depthResourceDescriptor.Format = DXGI_FORMAT_D32_FLOAT;
	depthResourceDescriptor.SampleDesc.Count = 1;
	depthResourceDescriptor.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	//�[�x�o�b�t�@�[�q�[�v�̐ݒ�
	D3D12_HEAP_PROPERTIES depthHeapProperty = {};
	depthHeapProperty.Type = D3D12_HEAP_TYPE_DEFAULT;
	depthHeapProperty.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	depthHeapProperty.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	//�N���A�o�����[�̐ݒ�
	D3D12_CLEAR_VALUE depthClearValue = {};
	depthClearValue.DepthStencil.Depth = 1.0f;  //�@�[�����ő�l�ŃN���A
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT; //32bit float�ŃN���A

	//�[�x�o�b�t�@�[�̍쐬
	result = _dev->CreateCommittedResource(
		&depthHeapProperty,
		D3D12_HEAP_FLAG_NONE,
		&depthResourceDescriptor,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&depthBuffer)
	);


	//�[�x�̂��߂̃f�B�X�N���v�^�q�[�v�̍쐬
	D3D12_DESCRIPTOR_HEAP_DESC depthStencilViewHeapDescriptor = {};
	depthStencilViewHeapDescriptor.NumDescriptors = 1;
	depthStencilViewHeapDescriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	result = _dev->CreateDescriptorHeap(&depthStencilViewHeapDescriptor, IID_PPV_ARGS(&dsvHeaps));

	//�[�x�r���[�̍쐬
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDescriptor = {};
	dsvDescriptor.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDescriptor.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDescriptor.Flags = D3D12_DSV_FLAG_NONE;
	_dev->CreateDepthStencilView(
		depthBuffer.Get(),
		&dsvDescriptor,
		dsvHeaps->GetCPUDescriptorHandleForHeapStart()
	);

	//�p�C�v���C���X�e�[�g�̍쐬�A�ݒ�
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};
	//�V�F�[�_�[��ݒ�
	gpipeline.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength = _vsBlob->GetBufferSize();
	gpipeline.PS.pShaderBytecode = _psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = _psBlob->GetBufferSize();
	//�T���v���}�X�N�ƃ��X�^���C�U�[�X�e�[�g��ݒ�
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	gpipeline.RasterizerState.MultisampleEnable = false;
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	gpipeline.RasterizerState.DepthClipEnable = true;
	//�u�����h�X�e�[�g�̐ݒ�
	gpipeline.BlendState.AlphaToCoverageEnable = false;
	gpipeline.BlendState.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;

	//���X�^���C�U�[�̐ݒ�
	gpipeline.RasterizerState.FrontCounterClockwise = false;
	gpipeline.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	gpipeline.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	gpipeline.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	gpipeline.RasterizerState.AntialiasedLineEnable = false;
	gpipeline.RasterizerState.ForcedSampleCount = 0;
	gpipeline.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	gpipeline.DepthStencilState.DepthEnable = true;
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL; //�[�x�o�b�t�@�[�ɐ[�x�l���������݂��ۂ��̎w��
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS; //�[�x�l�̏������ق����̗p����B�@�܂苗�����߂��@�H
	gpipeline.DepthStencilState.StencilEnable = false;
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	//���̓��C�A�E�g�̐ݒ�
	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);
	//�g���C�A���O���X�g���b�v�̐ݒ�
	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	//�v���~�e�B�u�g�|���W�̐ݒ�
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//�����_�[�^�[�Q�b�g�̐ݒ�
	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//�A���`�G�C���A�V���O�̃T���v�����̐ݒ�
	gpipeline.SampleDesc.Count = 1;
	gpipeline.SampleDesc.Quality = 0;

	//PSO�̐ݒ�̏I��


	//���[�g�V�O�l�`���ɐݒ肷�郋�[�g�p�����[�^�y�уf�B�X�N���v�^�e�[�u���A�f�B�X�N���v�^�����W�̐ݒ�
	//�P�ڂ̓V�F�[�_�[���\�[�X�r���[�̐ݒ�A�Q�ڂ͒萔�o�b�t�@�[�r���[�̐ݒ�
	D3D12_DESCRIPTOR_RANGE descTblRange[2] = {};
	descTblRange[0].NumDescriptors = 1;
	descTblRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descTblRange[0].BaseShaderRegister = 0;
	descTblRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descTblRange[1].NumDescriptors = 1;
	descTblRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descTblRange[1].BaseShaderRegister = 0;
	descTblRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//���[�g�p�����[�^�[�̐ݒ�
	D3D12_ROOT_PARAMETER rootparam{};

	rootparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootparam.DescriptorTable.NumDescriptorRanges = 2;
	rootparam.DescriptorTable.pDescriptorRanges = &descTblRange[0];


	/*
	D3D12_ROOT_PARAMETER rootparam[2] = {};

	rootparam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootparam[0].DescriptorTable.NumDescriptorRanges = 1;
	rootparam[0].DescriptorTable.pDescriptorRanges = &descTblRange[0];

	rootparam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootparam[1].DescriptorTable.NumDescriptorRanges = 1;
	rootparam[1].DescriptorTable.pDescriptorRanges = &descTblRange[1];
	*/

	//���[�g�V�O�l�`���ɐݒ肷��T���v���[�̐ݒ�
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};

	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc.Filter = D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

	//���[�g�V�O�l�`���̐ݒ�A����
	//���[�g�V�O�l�`���f�B�X�N���v�^�̐ݒ�
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//�f�B�X�N���v�^�e�[�u���̎��̂ł��郋�[�g�p�����[�^�[��ݒ�
	rootSignatureDesc.pParameters = &rootparam;
	rootSignatureDesc.NumParameters = 1;
	//�T���v���[��ݒ�
	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;

	//���[�g�V�O�l�`����Blob�̍쐬
	ID3DBlob* rootSigBlob = nullptr;

	result = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob,
		&errorBlob);

	//���[�g�V�O�l�`���I�u�W�F�N�g�̍쐬
	rootsignature = nullptr;
	result = _dev->CreateRootSignature(
		0,
		rootSigBlob->GetBufferPointer(),
		rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootsignature));

	rootSigBlob->Release();

	//�O���t�B�N�XPSO�I�u�W�F�N�g�̐���
	gpipeline.pRootSignature = rootsignature.Get();
	_pipelinestate = nullptr;
	result = _dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&_pipelinestate));

	//�r���[�|�[�g�̐ݒ�A����
	viewport.Width = window_width;
	viewport.Height = window_height;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;

	//�V�U�[��`�̐ݒ�A����
	scissorrect.top = 0;
	scissorrect.left = 0;
	scissorrect.right = scissorrect.left + window_width;
	scissorrect.bottom = scissorrect.top + window_height;


}

//�t���[���ɂ���čX�V����l������\��
void Dx12::OnUpdate()
{

	////�s��ϊ��p�s����t���[�����ɍX�V���|���S����Y���ŉ�]����悤�ɂ���B
	//angle += 0.1f;
	//worldMat = XMMatrixRotationY(angle);
	//*mapMatrix = worldMat * viewMat * projMat;

}


//�����_�����O����B���C�����[�v�̓���
void Dx12::OnRender() 
{
	//�R�}���h���X�g�Ɏ��ۂɎ��s���郌���_�����O�R�}���h���W�߂�
	PopulateCommandList();

	//�R�}���h���X�g�̖��߂̎��s

	ID3D12CommandList* cmdlists[] = { _cmdList.Get()};
	_cmdQueue->ExecuteCommandLists(1, cmdlists);

	//GPU�̃R�}���h���s�̓�����҂�
	WaitForPreviousFrame();

	HRESULT result = _cmdAllocator->Reset(); //�L���[�̃��Z�b�g
	result = _cmdList->Reset(_cmdAllocator.Get(), nullptr); //�ĂуR�}���h���X�g�����߂鏀��

	//��ʂ��X���b�v���`�悷��
	_swapchain->Present(1, 0);
}

//DirectX12���I������Ƃ��R�}���h���S�Ď��s����Ă���m�F
void Dx12::OnDestroy() 
{
	WaitForPreviousFrame();

	CloseHandle(_fenceevent);
}


//�R�}���h���X�g�Ɏ��ۂɎ��s����R�}���h��ǉ�
void Dx12::PopulateCommandList()
{


	D3D12_RESOURCE_BARRIER BarrierDesc = {};


	//auto result = _cmdAllocator->Reset();  //�R�}���h�A���P�[�^�[�̃��Z�b�g�B
	//�����_�[�^�[�Q�b�g���o�b�N�o�b�t�@�[�ɃZ�b�g
	auto bbIdx = _swapchain->GetCurrentBackBufferIndex();
	//���\�[�X�o���A�̐ݒ�
	//���\�[�X�o���A���o�b�N�o�b�t�@�[���\�[�X�Ɏw�肷��
	//�w���p�[�\���̂��g�p
	BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
		_backBuffers[bbIdx],
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	_cmdList->ResourceBarrier(1, &BarrierDesc);
	
	

	//�����_�[�^�[�Q�b�g���o�b�N�o�b�t�@�ɃZ�b�g 
	auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	auto dsvH = dsvHeaps->GetCPUDescriptorHandleForHeapStart();
	_cmdList->OMSetRenderTargets(1, &rtvH, true, &dsvH);

	//��ʃN���A
	float r, g, b;
	r = (float)(0xff & frame >> 4) / 255.0f;
	g = (float)(0xff & frame >> 8) / 255.0f;
	b = (float)(0xff & frame >> 0) / 255.0f;
	float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };// 
	_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
	++frame;

	//�[�x�o�b�t�@�[�N���A
	_cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);


	//�`�施��
	//PSO�ARootSignature,Primitive topology�̃Z�b�g
	_cmdList->SetPipelineState(_pipelinestate.Get());
	_cmdList->SetGraphicsRootSignature(rootsignature.Get());
	_cmdList->RSSetViewports(1, &viewport);
	_cmdList->RSSetScissorRects(1, &scissorrect);
	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//SRV�̃f�B�X�N���v�^�q�[�v�̎w��R�}���h���Z�b�g
	ID3D12DescriptorHeap* ppHeaps[] = { basicDescHeaps.Get() };
	_cmdList->SetDescriptorHeaps(1, ppHeaps);
	//���[�g�p�����^��srv�f�B�X�N���v�^�q�[�v�̃A�h���X�̊֘A�t��

	_cmdList->SetGraphicsRootDescriptorTable(0, basicDescHeaps->GetGPUDescriptorHandleForHeapStart());

	/*
	//���[�g�p�����^�ƒ萔�o�b�t�@�[�f�B�X�N���v�^�q�[�v�̃A�h���X�̊֘A�t��
	auto heapHandle = basicDescHeaps->GetGPUDescriptorHandleForHeapStart();
	heapHandle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	_cmdList->SetGraphicsRootDescriptorTable(1, heapHandle);
	*/

	//���_�o�b�t�@�[�̃Z�b�g
	_cmdList->IASetVertexBuffers(0, 1, &vbView);

	//�C���f�b�N�X�o�b�t�@�[�̃Z�b�g
	_cmdList->IASetIndexBuffer(&ibView);

	//���ۂ̕`�施��
	//_cmdList->DrawInstanced(vertNum, 1, 0, 0); //���_�C���f�b�N�X��g�p
	_cmdList->DrawIndexedInstanced(indicesNum, 1, 0, 0, 0); //���_�C���f�b�N�X�g�p


	//���\�[�X�o���A�̏�Ԃ̐ݒ�
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	_cmdList->ResourceBarrier(1, &BarrierDesc);

	

	_cmdList->Close();
}


//GPU���R�}���h��S�Ď��s��������܂ő҂�
void Dx12::WaitForPreviousFrame()
{
	//���߂̊�����҂��A�`�F�b�N
	//TODO: ����Signal()���\�b�h�ɂ������͒P���Ȃ̂ő��̕��@���l����
	_cmdQueue->Signal(_fence.Get(), ++_fenceVal);
	if (_fence->GetCompletedValue() != _fenceVal)
	{
		//�C�x���g�n���h���̎擾
		auto event = CreateEvent(nullptr, false, false, nullptr);

		_fence->SetEventOnCompletion(_fenceVal, event);

		//�C�x���g����������܂ő҂�������
		WaitForSingleObject(event, INFINITE);

		CloseHandle(event);
	}
}