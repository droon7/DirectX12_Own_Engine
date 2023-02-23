#include"DirectX12_Tutorial.h"

using namespace DirectX;


//WIC�e�N�X�`���̃��[�h
	ComPtr<ID3D12Resource> Dx12::LoadTextureFromFile(std::string& texPath)
{
	metadata = {};
	scratchImg = {};

	auto result = LoadFromWICFile(
		GetWideStringFromString(texPath).c_str(),
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


	ComPtr<ID3D12Resource> texbuff = nullptr;
	//�R�s�[��̃e�N�X�`���o�b�t�@�[�̍쐬
	result = _dev->CreateCommittedResource(
		&textureHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&textureBuffDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&texbuff)
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

		ID3D12CommandList* cmdlists[] = { _cmdList.Get() };
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

	return texbuff;
}