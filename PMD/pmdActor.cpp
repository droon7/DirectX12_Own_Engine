#include"PmdActor.h"
#include"Utility.h"

void* Transform::operator new(size_t size)
{
	return _aligned_malloc(size, 16);
}

//コンストラクタでロードからビュー作成
//モデルの初期値設定
PmdActor::PmdActor(DX12Application* app, std::string ModelName, std::string motionPath, int x)
	:angle(0.0f), stringModelPath(ModelName)
{
	//ロードからビュー作成
	LoadPmdData(ModelName);
	pmdBone = PmdBone(pmdData.pmdBoneDatas, motionPath);
	CreateVertexViewIndexView(app);
	SetTransform(x,0,0);
	CreateTransformView(app);
	GetMaterialResource(app);
	GetTextureResource(app);
	CreateMaterialAndTextureView(app);

	//モデルの初期値
	
	SetPmdBone(0);
	PlayAnimation();

	//デバッグ用
	//ShowIkBoneDebug();
}
//ここからロードとDirectX12のための処理
void PmdActor::LoadPmdData(std::string ModelName)
{
	pmdData.LoadPmdData(ModelName);
}

void PmdActor::CreateVertexViewIndexView(DX12Application* app)
{


	//頂点バッファーの生成
	D3D12_HEAP_PROPERTIES heapprop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC resourcedesc = CD3DX12_RESOURCE_DESC::Buffer(pmdData.vertices.size());

	vertBuff = nullptr;
	auto result = app->_dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resourcedesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(vertBuff.ReleaseAndGetAddressOf())
	);

	//MapメソッドでビデオメモリvertBuff上に頂点を書き込む
	unsigned char* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap); //vertMap上にvertBuffの仮想メモリを置く
	std::copy(std::begin(pmdData.vertices), std::end(pmdData.vertices), vertMap);
	vertBuff->Unmap(0, nullptr);  //仮想メモリを解除

	//頂点バッファービューの生成
	vbView = {};
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = static_cast<UINT>(pmdData.vertices.size());
	vbView.StrideInBytes = pmdData.pmdvertex_size;

	//インデックスバッファーの生成
	idxBuff = nullptr;

	resourcedesc = CD3DX12_RESOURCE_DESC::Buffer(pmdData.indices.size() * sizeof(pmdData.indices[0]));

	result = app->_dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resourcedesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(idxBuff.ReleaseAndGetAddressOf())
	);

	unsigned short* mappedIdx = nullptr;
	idxBuff->Map(0, nullptr, (void**)&mappedIdx);

	std::copy(std::begin(pmdData.indices), std::end(pmdData.indices), mappedIdx);
	idxBuff->Unmap(0, nullptr);

	//インデックスバッファービューの作成
	ibView = {};

	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = static_cast<UINT>(pmdData.indices.size() * sizeof(pmdData.indices[0]));

}

//初期位置セット
void PmdActor::SetTransform(int x, int y, int z)
{
	transform.worldMatrix = DirectX::XMMatrixIdentity();
	transform.worldMatrix = DirectX::XMMatrixTranslation(x, y, z);
}

//PMDモデルの位置変換行列のビューの作成、ワールド行列とボーン変換行列
HRESULT PmdActor::CreateTransformView(DX12Application* app)
{
	//定数バッファーの作成
	auto buffSize = sizeof(DirectX::XMMATRIX) * (1 + pmdBone.boneMatrices.size());
	buffSize = (buffSize + 0xff) & ~0xff;

	auto constHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto constHeapDesc = CD3DX12_RESOURCE_DESC::Buffer(buffSize);

	auto result = app->_dev->CreateCommittedResource(
		&constHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&constHeapDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(transformBuff.ReleaseAndGetAddressOf())
	);

	if (FAILED(result))
	{
		assert(SUCCEEDED(result));
		return result;
	}
	
	//マップによる定数の転送
	result = transformBuff->Map(0, nullptr, (void**)&mapTransform);
	if (FAILED(result))
	{
		assert(SUCCEEDED(result));
		return result;
	}

	mapTransform[0] = transform.worldMatrix;
	std::copy(pmdBone.boneMatrices.cbegin(), pmdBone.boneMatrices.cend(), mapTransform + 1);

	//定数バッファービューの作成のための設定
	//行列用定数バッファービュー用のディスクリプタヒープの作成
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	//シェーダーから見えるように
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = 1;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	result = app->_dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(transformDescHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		assert(SUCCEEDED(result));
		return result;
	}

	auto matrixHeaphandle = transformDescHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_CONSTANT_BUFFER_VIEW_DESC constBufferViewDesc = {};
	constBufferViewDesc.BufferLocation = transformBuff->GetGPUVirtualAddress();
	constBufferViewDesc.SizeInBytes = static_cast<UINT>(transformBuff->GetDesc().Width);

	//定数バッファービューの作成
	app->_dev->CreateConstantBufferView(
		&constBufferViewDesc,
		matrixHeaphandle
	);

	return S_OK;
}
//PMDの情報からGPUへのマテリアルリソースのロード
void PmdActor::GetMaterialResource(DX12Application* app)
{
	//マテリアルバッファーの作成、バッファーサイズを256バイトでアライメントする
	auto materialBuffSize = sizeof(MaterialForHlsl);
	materialBuffSize = (materialBuffSize + 0xff) & ~0xff;

	auto materialHeapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto materialResourceDescriptor = CD3DX12_RESOURCE_DESC::Buffer(materialBuffSize * pmdData.materialNum);

	auto result = app->_dev->CreateCommittedResource(
		&materialHeapProperty,
		D3D12_HEAP_FLAG_NONE,
		&materialResourceDescriptor,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(materialBuff.ReleaseAndGetAddressOf())
	);

	char* mapMaterial = nullptr;

	result = materialBuff->Map(0, nullptr, (void**)&mapMaterial);

	//char*をmaterialForHlsl*型に変換
	for (auto& m : pmdData.materialDatas) {
		*reinterpret_cast<MaterialForHlsl*>(mapMaterial) = m.material;
		mapMaterial += materialBuffSize;
	}
	materialBuff->Unmap(0, nullptr);
}

void PmdActor::GetTextureResource(DX12Application* app)
{
	//マテリアルの数だけテクスチャをロードする。対応するテクスチャがなければnullptrを入れる。
	//テクスチャ名にセパレーターがあれば分離し、適切な名前を入れる
	pmdTexture.textureResources.resize(pmdData.materialNum);
	pmdTexture.sphResources.resize(pmdData.materialNum);
	pmdTexture.spaResources.resize(pmdData.materialNum);

	for (int i = 0; i < static_cast<signed int>(pmdData.materialNum); ++i)
	{
		std::string texFileName = pmdData.materialDatas[i].additional.texPath;
		std::string sphFileName = {};
		std::string spaFileName = {};

		if (texFileName.size() == 0)
		{
			pmdTexture.textureResources[i] = nullptr;
			continue;
		}

		{
			auto namepair = SplitFileName(texFileName, '*');
			if (GetExtension(namepair.second) == "sph")
			{
				sphFileName = namepair.second;
				texFileName = namepair.first;
			}
			else if (GetExtension(namepair.second) == "spa")
			{
				spaFileName = namepair.second;
				texFileName = namepair.first;
			}
			else
			{
				texFileName = namepair.second;
			}
		}

		auto texFilePath = GetTexturePathFromModelAndTexPath(
			stringModelPath,
			texFileName.c_str());

		auto sphFilePath = GetTexturePathFromModelAndTexPath(
			stringModelPath,
			sphFileName.c_str());
		auto spaFilePath = GetTexturePathFromModelAndTexPath(
			stringModelPath,
			spaFileName.c_str());

		pmdTexture.textureResources[i] =pmdTexture.LoadTextureFromFile(texFilePath,app);
		pmdTexture.sphResources[i] =pmdTexture.LoadTextureFromFile(sphFilePath, app);
		pmdTexture.spaResources[i] =pmdTexture.LoadTextureFromFile(spaFilePath, app);

	}

	//トゥーンシェーダーのためのカラールックアップテーブルのロード
	pmdTexture.toonResources.resize(pmdData.materialDatas.size());

	for (int i = 0; i < pmdData.materialDatas.size(); ++i)
	{
		std::string toonFilePath = "toon/";
		char toonFileName[16];

		sprintf_s(
			toonFileName,
			16,
			"toon%02d.bmp",
			pmdData.materialDatas[i].additional.toonIdx + 1
		);

		toonFilePath += toonFileName;

		pmdTexture.toonResources[i] =pmdTexture.LoadTextureFromFile(toonFilePath, app);
	}




}

void PmdActor::CreateMaterialAndTextureView(DX12Application* app)
{
	//マテリアルとテクスチャ用のディスクリプタヒープの作成。
	D3D12_DESCRIPTOR_HEAP_DESC DescHeapDesc = {};
	DescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	DescHeapDesc.NodeMask = 0;
	DescHeapDesc.NumDescriptors = pmdData.materialNum * 5;
	DescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	auto result = app->_dev->CreateDescriptorHeap(
		&DescHeapDesc, IID_PPV_ARGS(materialDescHeap.ReleaseAndGetAddressOf())
	);

	//シェーダーリソースビューディスクリプタの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	//マテリアル用CBV、SRVの作成、マテリアルの数だけ作る。
	auto materialBuffSize = sizeof(MaterialForHlsl);
	materialBuffSize = (materialBuffSize + 0xff) & ~0xff;
	D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = {};
	matCBVDesc.BufferLocation = materialBuff->GetGPUVirtualAddress();
	matCBVDesc.SizeInBytes = static_cast<UINT>(materialBuffSize);

	auto matDescHeapHead = materialDescHeap->GetCPUDescriptorHandleForHeapStart(); //先頭を記録
	auto inc = app->_dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	whiteTex = pmdTexture.CreateWhiteTexture(app);
	blackTex = pmdTexture.CreateBlackTexture(app);
	gradTex = pmdTexture.CreateGradationTexture(app);

	//以下実際にCBV、SRVを作る。
	for (int i = 0; i < static_cast<signed int>(pmdData.materialNum); ++i)
	{
		app->_dev->CreateConstantBufferView(&matCBVDesc, matDescHeapHead);
		matDescHeapHead.ptr += inc;
		matCBVDesc.BufferLocation += materialBuffSize;

		if (pmdTexture.textureResources[i] != nullptr)		//テクスチャがあればそのテクスチャ、なければ白テクスチャを設定
		{
			srvDesc.Format = pmdTexture.textureResources[i]->GetDesc().Format;
			app->_dev->CreateShaderResourceView(
				pmdTexture.textureResources[i].Get(),
				&srvDesc,
				matDescHeapHead
			);
		}
		else
		{
			srvDesc.Format = whiteTex->GetDesc().Format;
			app->_dev->CreateShaderResourceView(
				whiteTex.Get(),
				&srvDesc,
				matDescHeapHead
			);

		}
		matDescHeapHead.ptr += inc;

		if (pmdTexture.sphResources[i] != nullptr)		//SPHがあればそのSPH、なければ白テクスチャを設定
		{
			srvDesc.Format = pmdTexture.sphResources[i]->GetDesc().Format;
			app->_dev->CreateShaderResourceView(
				pmdTexture.sphResources[i].Get(),
				&srvDesc,
				matDescHeapHead
			);
		}
		else
		{
			srvDesc.Format = whiteTex->GetDesc().Format;
			app->_dev->CreateShaderResourceView(
				whiteTex.Get(),
				&srvDesc,
				matDescHeapHead
			);

		}
		matDescHeapHead.ptr += inc;

		if (pmdTexture.spaResources[i] != nullptr)		//SPAがあればそのSPA、なければ黒テクスチャを設定
		{
			srvDesc.Format = pmdTexture.spaResources[i]->GetDesc().Format;
			app->_dev->CreateShaderResourceView(
				pmdTexture.spaResources[i].Get(),
				&srvDesc,
				matDescHeapHead
			);
		}
		else
		{
			srvDesc.Format = blackTex->GetDesc().Format;
			app->_dev->CreateShaderResourceView(
				blackTex.Get(),
				&srvDesc,
				matDescHeapHead
			);

		}

		matDescHeapHead.ptr += inc;

		if (pmdTexture.toonResources[i] != nullptr)	//トゥーンリソースがあればトゥーン用、なければグラデーションテクスチャを設定
		{
			srvDesc.Format = pmdTexture.toonResources[i]->GetDesc().Format;
			app->_dev->CreateShaderResourceView(
				pmdTexture.toonResources[i].Get(),
				&srvDesc,
				matDescHeapHead
			);
		}
		else
		{
			srvDesc.Format = gradTex->GetDesc().Format;
			app->_dev->CreateShaderResourceView(
				gradTex.Get(),
				&srvDesc,
				matDescHeapHead
			);

		}
		matDescHeapHead.ptr += inc;

	}
}

//ここまでDirectX12の内部処理


void PmdActor::DrawPmd(DX12Application* app)
{
	//頂点バッファーのセット
	app->_cmdList->IASetVertexBuffers(0, 1, &vbView);
	//インデックスバッファーのセット
	app->_cmdList->IASetIndexBuffer(&ibView);

	//ワールド＋変換行列定数ヒープのセット
	ID3D12DescriptorHeap* transheaps[] = { transformDescHeap.Get() };
	app->_cmdList->SetDescriptorHeaps(1, transheaps);
	app->_cmdList->SetGraphicsRootDescriptorTable(1, transformDescHeap->GetGPUDescriptorHandleForHeapStart());

	//マテリアルディスクリプタヒープのセット
	ID3D12DescriptorHeap* ppHeaps1[] = { materialDescHeap.Get() };
	app->_cmdList->SetDescriptorHeaps(1, ppHeaps1);

	//マテリアルのディスクリプタテーブルのセットとそれに対応したインデッックスを更新しながら描画していく。
	//その上さらにテクスチャ、sph、spa、トゥーンテクスチャも描画していく。
	auto materialH = materialDescHeap->GetGPUDescriptorHandleForHeapStart();
	auto cbvSrvIncSize = app->_dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 5;

	unsigned int idxOffset = 0;
	for (auto& m : pmdData.materialDatas)
	{
		app->_cmdList->SetGraphicsRootDescriptorTable(2, materialH);

		app->_cmdList->DrawIndexedInstanced(m.indicesNum, 1, idxOffset, 0, 0);

		materialH.ptr += cbvSrvIncSize;

		idxOffset += m.indicesNum;
	}
}


void PmdActor::UpdatePmd()
{
	DWORD elapsedTime = timeGetTime() - startTime;
	unsigned int motionDuration = pmdBone.GetMotionDataDuration();
	unsigned int frameNo = static_cast<unsigned int>(30 * (elapsedTime / 1000.0f));//% vmdData.duration;

	//ループアニメーションのための時間変数とフレームの初期化
	if (frameNo > motionDuration+5)
	{
		startTime = timeGetTime();
		frameNo = 0;
	}

	SetPmdBone(frameNo);
	//angle += 0.01f;
	//mapTransform[0] = DirectX::XMMatrixRotationY(angle);

}


void PmdActor::SetPmdBone(unsigned int frameNo)
{
	pmdBone.SetBoneMatrices(frameNo);
	std::copy(pmdBone.boneMatrices.cbegin(), pmdBone.boneMatrices.cend(), mapTransform + 1);
}


void PmdActor::PlayAnimation()
{
	startTime = timeGetTime();
}

//デバッグ用関数

//void PmdActor::ShowIkBoneDebug()
//{
//	auto getNameFromIdx = [&](uint16_t idx) -> std::string
//	{
//		auto it = find_if(pmdBone.boneNodeTable.begin(),
//			pmdBone.boneNodeTable.end(),
//			[&](const std::pair<std::string, BoneNode>& obj)
//			{
//				return obj.second.boneIdx == idx;
//			}
//		);
//
//		if (it != pmdBone.boneNodeTable.end())
//		{
//			return it->first;
//		}
//		else
//		{
//			return "";
//		}
//	};
//
//
//	for (auto& ik : pmdData.pmdIkData)
//	{
//		std::ostringstream oss;
//		oss << "IKボーン番号=" << ik.boneIdx
//			<< ":" << getNameFromIdx(ik.boneIdx) << std::endl;
//
//		for (auto& node : ik.nodeIdx)
//		{
//			oss << "\tノードボーン=" << node
//				<< ":" << getNameFromIdx(node) << std::endl;
//		}
//		OutputDebugStringA(oss.str().c_str());
//	}
//
//}


