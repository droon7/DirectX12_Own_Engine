#include "PmdTexture.h"
#include "Utility.h"

PmdTexture::PmdTexture()
{
}


//テクスチャの名前を受け取りテクスチャデータをロードし返す
ComPtr<ID3D12Resource> PmdTexture::LoadTextureFromFile(std::string& textureName, DX12Application* app)
{

	//flyweightパターンの仕様により同じリソースを再ロードせず再利用する
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

	//テクスチャの中間バッファーとしてのアップロードヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProp = {};
	uploadHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	//UPLOAD用なのでUNKNOWNに設定
	uploadHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	uploadHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	uploadHeapProp.CreationNodeMask = 0;
	uploadHeapProp.VisibleNodeMask = 0;

	//テクスチャの中間バッファリソースディスクリプタの設定
	//中間バッファーなのでテクスチャとして指定しない
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

	//テクスチャの中間バッファーの作成
	ComPtr<ID3D12Resource> uploadbuff = nullptr;

	result = app->_dev->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&textureBuffDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadbuff.ReleaseAndGetAddressOf())
	);

	//中間バッファからのコピー先のテクスチャバッファーの作成

	D3D12_HEAP_PROPERTIES textureHeapProp = {};

	//テクスチャのヒープの設定
	textureHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	textureHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	textureHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	textureHeapProp.CreationNodeMask = 0;
	textureHeapProp.VisibleNodeMask = 0;

	//テクスチャのリソースの設定
	textureBuffDesc.Format = metadata.format;
	textureBuffDesc.Width = static_cast<UINT16>(metadata.width);
	textureBuffDesc.Height = static_cast<UINT>(metadata.height);
	textureBuffDesc.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize);
	textureBuffDesc.MipLevels = static_cast<UINT>(metadata.mipLevels);
	textureBuffDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
	textureBuffDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;


	ComPtr<ID3D12Resource> texbuff = nullptr;
	//コピー先のテクスチャバッファーの作成
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

	//中間バッファーへテクスチャデータをコピー
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


	//CommandList::CopyTextureRegion()の引数の構造体を作る
	//アップロード側の作成
	D3D12_TEXTURE_COPY_LOCATION src;
	src.pResource = uploadbuff.Get();
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint.Offset = 0;
	src.PlacedFootprint.Footprint.Width = static_cast<UINT>(metadata.width);
	src.PlacedFootprint.Footprint.Height = static_cast<UINT>(metadata.height);
	src.PlacedFootprint.Footprint.Depth = static_cast<UINT>(metadata.depth);
	src.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
	src.PlacedFootprint.Footprint.Format = img->format;

	//コピー先の設定
	D3D12_TEXTURE_COPY_LOCATION dst;
	dst.pResource = texbuff.Get();
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;

	app->_cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	{

		//テクスチャのコピーのためのリソースバリアの設定
		//TODO:関数化したい

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

	//flyweightパターンのための追加
	_resourceTable.insert(std::make_pair(textureName, texbuff));

	return texbuff;
}


//白テクスチャを作り返す。注意！WriteToSubresourceメソッドを使用
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
		static_cast<UINT>(data.size())
	);

	return whiteBuff;
}

//黒テクスチャを作り返す。注意！WriteToSubresourceメソッドを使用
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
		static_cast<UINT>(data.size())
	);

	return blackBuff;
}

//トゥーンシェーダーしない場合のデフォルトグラデーションテクスチャを作り返す。注意！WriteToSubresourceメソッドを使用
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

	//32bitに対して、左8bitは0xff、そこから8bitずつはRGBマクロで0xffから1ずつ減らしていく
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
		static_cast <UINT>(sizeof(unsigned int) * data.size())
	);

	return gradBuff;
}
