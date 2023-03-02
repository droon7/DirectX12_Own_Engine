#include "PmdTexture.h"

PmdTexture::PmdTexture()
{
}


//�e�N�X�`���̖��O���󂯎��e�N�X�`���f�[�^�����[�h���Ԃ�
ComPtr<ID3D12Resource> PmdTexture::LoadTextureFromFile(std::string& textureName, DX12Application* app)
{

	//flyweight�p�^�[���̎d�l�ɂ�蓯�����\�[�X���ă��[�h�����ė��p����
	auto iterator = _resourceTable.find(textureName);
	if (iterator != _resourceTable.end())
	{
		return iterator->second;
	}


	DirectX::TexMetadata metadata;
	DirectX::ScratchImage scratchImg;

	auto result = LoadFromWICFile(
		GetWideStringFromString(textureName).c_str(),
		DirectX::WIC_FLAGS_NONE,
		&metadata,
		scratchImg
	);

	if (FAILED(result))
	{
		return nullptr;
	}

	auto img = scratchImg.GetImage(0, 0, 0);

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
	ComPtr<ID3D12Resource> uploadbuff = nullptr;

	result = app->_dev->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&textureBuffDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadbuff.ReleaseAndGetAddressOf())
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


	ComPtr<ID3D12Resource> texbuff = nullptr;
	//�R�s�[��̃e�N�X�`���o�b�t�@�[�̍쐬
	result = app->_dev->CreateCommittedResource(
		&textureHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&textureBuffDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(texbuff.ReleaseAndGetAddressOf())
	);

	if (FAILED(result))
	{
		return nullptr;
	}

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
	D3D12_TEXTURE_COPY_LOCATION src;
	src.pResource = uploadbuff.Get();
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint.Offset = 0;
	src.PlacedFootprint.Footprint.Width = metadata.width;
	src.PlacedFootprint.Footprint.Height = metadata.height;
	src.PlacedFootprint.Footprint.Depth = metadata.depth;
	src.PlacedFootprint.Footprint.RowPitch = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	src.PlacedFootprint.Footprint.Format = img->format;

	//�R�s�[��̐ݒ�
	D3D12_TEXTURE_COPY_LOCATION dst;
	dst.pResource = texbuff.Get();
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;

	app->_cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	{

		//�e�N�X�`���̃R�s�[�̂��߂̃��\�[�X�o���A�̐ݒ�
		//TODO:�֐���������

		D3D12_RESOURCE_BARRIER BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
			texbuff.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
		);
		app->_cmdList->ResourceBarrier(1, &BarrierDesc);

		app->_cmdList->Close();

		ID3D12CommandList* cmdlists[] = { app->_cmdList.Get() };
		app->_cmdQueue->ExecuteCommandLists(1, cmdlists);

		app->_cmdQueue->Signal(app->_fence.Get(), ++app->_fenceVal);

		if (app->_fence->GetCompletedValue() != app->_fenceVal)
		{
			auto event = CreateEvent(nullptr, false, false, nullptr);
			app->_fence->SetEventOnCompletion(app->_fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}
		app->_cmdAllocator->Reset();
		app->_cmdList->Reset(app->_cmdAllocator.Get(), nullptr);
	}

	//flyweight�p�^�[���̂��߂̒ǉ�
	_resourceTable.insert(std::make_pair(textureName, texbuff));

	return texbuff;
}


//���e�N�X�`�������Ԃ��B���ӁIWriteToSubresource���\�b�h���g�p
ComPtr<ID3D12Resource> PmdTexture::CreateWhiteTexture(DX12Application* app)
{
	D3D12_HEAP_PROPERTIES textureHeapProperty = CD3DX12_HEAP_PROPERTIES(
		D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
		D3D12_MEMORY_POOL_L0);

	D3D12_RESOURCE_DESC resourceDescriptor = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, 4, 4);

	ComPtr<ID3D12Resource> whiteBuff = nullptr;

	auto result = app->_dev->CreateCommittedResource(
		&textureHeapProperty,
		D3D12_HEAP_FLAG_NONE,
		&resourceDescriptor,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(whiteBuff.ReleaseAndGetAddressOf())
	);

	if (FAILED(result)) {
		return nullptr;
	}
	std::vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), 0xff);

	result = whiteBuff->WriteToSubresource(
		0,
		nullptr,
		data.data(),
		4 * 4,
		data.size()
	);

	return whiteBuff;
}

//���e�N�X�`�������Ԃ��B���ӁIWriteToSubresource���\�b�h���g�p
ComPtr<ID3D12Resource> PmdTexture::CreateBlackTexture(DX12Application* app)
{
	D3D12_HEAP_PROPERTIES textureHeapProperty = CD3DX12_HEAP_PROPERTIES(
		D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
		D3D12_MEMORY_POOL_L0);

	D3D12_RESOURCE_DESC resourceDescriptor = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, 4, 4);

	ComPtr<ID3D12Resource> blackBuff = nullptr;

	auto result = app->_dev->CreateCommittedResource(
		&textureHeapProperty,
		D3D12_HEAP_FLAG_NONE,
		&resourceDescriptor,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(blackBuff.ReleaseAndGetAddressOf())
	);

	if (FAILED(result)) {
		return nullptr;
	}
	std::vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), 0x0);

	result = blackBuff->WriteToSubresource(
		0,
		nullptr,
		data.data(),
		4 * 4,
		data.size()
	);

	return blackBuff;
}

//�g�D�[���V�F�[�_�[���Ȃ��ꍇ�̃f�t�H���g�O���f�[�V�����e�N�X�`�������Ԃ��B���ӁIWriteToSubresource���\�b�h���g�p
ComPtr<ID3D12Resource> PmdTexture::CreateGradationTexture(DX12Application* app)
{
	D3D12_HEAP_PROPERTIES textureHeapProperty = CD3DX12_HEAP_PROPERTIES(
		D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
		D3D12_MEMORY_POOL_L0);

	D3D12_RESOURCE_DESC resourceDescriptor = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, 4, 256);

	ComPtr<ID3D12Resource> gradBuff = nullptr;

	auto result = app->_dev->CreateCommittedResource(
		&textureHeapProperty,
		D3D12_HEAP_FLAG_NONE,
		&resourceDescriptor,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(gradBuff.ReleaseAndGetAddressOf())
	);

	if (FAILED(result)) {
		return nullptr;
	}

	//32bit�ɑ΂��āA��8bit��0xff�A��������8bit����RGB�}�N����0xff����1�����炵�Ă���
	std::vector<unsigned int> data(4 * 256);
	auto iterator = data.begin();
	unsigned int c = 0xff;
	for (; iterator != data.end(); iterator += 4)
	{
		auto col = (0xff << 24) | RGB(c, c, c);
		std::fill(iterator, iterator + 4, col);
		--c;
	}


	result = gradBuff->WriteToSubresource(
		0,
		nullptr,
		data.data(),
		4 * sizeof(unsigned int),
		sizeof(unsigned int) * data.size()
	);

	return gradBuff;
}
