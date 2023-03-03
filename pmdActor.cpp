#include"PmdActor.h"

void* Transform::operator new(size_t size)
{
	return _aligned_malloc(size, 16);
}

//�R���X�g���N�^�Ń��[�h����r���[�쐬�܂ōs��
PmdActor::PmdActor(DX12Application* app, std::string ModelName)
	:angle(0.0f)
{
	LoadPmdData(ModelName);
	CreateVertexViewIndexView(app);
	CreateTransformView(app);
	GetMaterialResource(app);
	GetTextureResource(app);
	CreateMaterialAndTextureView(app);
}

void PmdActor::LoadPmdData(std::string ModelName)
{
	pmdData.loadPmdData(ModelName);
}

void PmdActor::CreateVertexViewIndexView(DX12Application* app)
{


	//���_�o�b�t�@�[�̐���
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

	//Map���\�b�h�Ńr�f�I������vertBuff��ɒ��_����������
	unsigned char* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap); //vertMap���vertBuff�̉��z��������u��
	std::copy(std::begin(pmdData.vertices), std::end(pmdData.vertices), vertMap);
	vertBuff->Unmap(0, nullptr);  //���z������������

	//���_�o�b�t�@�[�r���[�̐���
	vbView = {};
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = pmdData.vertices.size();
	vbView.StrideInBytes = pmdData.pmdvertex_size;

	//�C���f�b�N�X�o�b�t�@�[�̐���
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

	//�C���f�b�N�X�o�b�t�@�[�r���[�̍쐬
	ibView = {};

	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = static_cast<UINT>(pmdData.indices.size() * sizeof(pmdData.indices[0]));

}

void PmdActor::SetTransform()
{
	transform.worldMatrix = DirectX::XMMatrixRotationY(0);
}

//PMD���f���̈ʒu�ϊ��s��̃r���[�̍쐬
void PmdActor::CreateTransformView(DX12Application* app)
{
	//�萔�o�b�t�@�[�̍쐬
	auto constHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto constHeapDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(SceneMatrix) + 0xff) & ~0xff);

	app->_dev->CreateCommittedResource(
		&constHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&constHeapDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(transformBuff.ReleaseAndGetAddressOf())
	);
	Transform mapMatrix;

	//�}�b�v�ɂ��萔�̓]��
	auto result = transformBuff->Map(0, nullptr, (void**)&mapMatrix);
	mapMatrix.worldMatrix = transform.worldMatrix;

	//�萔�o�b�t�@�[�r���[�̍쐬�̂��߂̐ݒ�
	//�s��p�萔�o�b�t�@�[�r���[�p�̃f�B�X�N���v�^�q�[�v�̍쐬
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	//�V�F�[�_�[���猩����悤��
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = 1;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	result = app->_dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(transformDescHeap.ReleaseAndGetAddressOf()));

	auto matrixHeaphandle = transformDescHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_CONSTANT_BUFFER_VIEW_DESC constBufferViewDesc = {};
	constBufferViewDesc.BufferLocation = transformBuff->GetGPUVirtualAddress();
	constBufferViewDesc.SizeInBytes = transformBuff->GetDesc().Width;

	//�萔�o�b�t�@�[�r���[�̍쐬
	app->_dev->CreateConstantBufferView(
		&constBufferViewDesc,
		matrixHeaphandle
	);

}

//PMD�̏�񂩂�GPU�ւ̃}�e���A�����\�[�X�̃��[�h
void PmdActor::GetMaterialResource(DX12Application* app)
{
	//�}�e���A���o�b�t�@�[�̍쐬�A�o�b�t�@�[�T�C�Y��256�o�C�g�ŃA���C�����g����
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

	//char*��materialForHlsl*�^�ɕϊ�
	for (auto& m : pmdData.materialDatas) {
		*reinterpret_cast<MaterialForHlsl*>(mapMaterial) = m.material;
		mapMaterial += materialBuffSize;
	}
	materialBuff->Unmap(0, nullptr);
}

void PmdActor::GetTextureResource(DX12Application* app)
{
	//�}�e���A���̐������e�N�X�`�������[�h����B�Ή�����e�N�X�`�����Ȃ����nullptr������B
	//�e�N�X�`�����ɃZ�p���[�^�[������Ε������A�K�؂Ȗ��O������
	pmdTexture.textureResources.resize(pmdData.materialNum);
	pmdTexture.sphResources.resize(pmdData.materialNum);
	pmdTexture.spaResources.resize(pmdData.materialNum);

	for (int i = 0; i < pmdData.materialNum; ++i)
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

	//�g�D�[���V�F�[�_�[�̂��߂̃J���[���b�N�A�b�v�e�[�u���̃��[�h
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
	//�}�e���A���ƃe�N�X�`���p�̃f�B�X�N���v�^�q�[�v�̍쐬�B
	D3D12_DESCRIPTOR_HEAP_DESC DescHeapDesc = {};
	DescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	DescHeapDesc.NodeMask = 0;
	DescHeapDesc.NumDescriptors = pmdData.materialNum * 5;
	DescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	auto result = app->_dev->CreateDescriptorHeap(
		&DescHeapDesc, IID_PPV_ARGS(materialDescHeap.ReleaseAndGetAddressOf())
	);

	//�V�F�[�_�[���\�[�X�r���[�f�B�X�N���v�^�̍쐬
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	//�}�e���A���pCBV�ASRV�̍쐬�A�}�e���A���̐��������B
	auto materialBuffSize = sizeof(MaterialForHlsl);
	materialBuffSize = (materialBuffSize + 0xff) & ~0xff;
	D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = {};
	matCBVDesc.BufferLocation = materialBuff->GetGPUVirtualAddress();
	matCBVDesc.SizeInBytes = materialBuffSize;

	auto matDescHeapHead = materialDescHeap->GetCPUDescriptorHandleForHeapStart(); //�擪���L�^
	auto inc = app->_dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	ComPtr<ID3D12Resource> whiteTex = pmdTexture.CreateWhiteTexture(app);
	ComPtr<ID3D12Resource> blackTex = pmdTexture.CreateBlackTexture(app);
	ComPtr<ID3D12Resource> gradTex = pmdTexture.CreateGradationTexture(app);

	//�ȉ����ۂ�CBV�ASRV�����B
	for (int i = 0; i < pmdData.materialNum; ++i)
	{
		app->_dev->CreateConstantBufferView(&matCBVDesc, matDescHeapHead);
		matDescHeapHead.ptr += inc;
		matCBVDesc.BufferLocation += materialBuffSize;

		if (pmdTexture.textureResources[i] != nullptr)		//�e�N�X�`��������΂��̃e�N�X�`���A�Ȃ���Δ��e�N�X�`����ݒ�
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

		if (pmdTexture.sphResources[i] != nullptr)		//SPH������΂���SPH�A�Ȃ���Δ��e�N�X�`����ݒ�
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

		if (pmdTexture.spaResources[i] != nullptr)		//SPA������΂���SPA�A�Ȃ���΍��e�N�X�`����ݒ�
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

		if (pmdTexture.toonResources[i] != nullptr)	//�g�D�[�����\�[�X������΃g�D�[���p�A�Ȃ���΃O���f�[�V�����e�N�X�`����ݒ�
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

void PmdActor::DrawPmd(DX12Application* app)
{
	//���_�o�b�t�@�[�̃Z�b�g
	app->_cmdList->IASetVertexBuffers(0, 1, &vbView);
	//�C���f�b�N�X�o�b�t�@�[�̃Z�b�g
	app->_cmdList->IASetIndexBuffer(&ibView);

	//���[���h�{�ϊ��s��萔�q�[�v�̃Z�b�g
	ID3D12DescriptorHeap* transheaps[] = { transformDescHeap.Get() };
	app->_cmdList->SetDescriptorHeaps(1, transheaps);
	app->_cmdList->SetGraphicsRootDescriptorTable(1, transformDescHeap->GetGPUDescriptorHandleForHeapStart());

	//�}�e���A���f�B�X�N���v�^�q�[�v�̃Z�b�g
	ID3D12DescriptorHeap* ppHeaps1[] = { materialDescHeap.Get() };
	app->_cmdList->SetDescriptorHeaps(1, ppHeaps1);

	//�}�e���A���̃f�B�X�N���v�^�e�[�u���̃Z�b�g�Ƃ���ɑΉ������C���f�b�b�N�X���X�V���Ȃ���`�悵�Ă����B
	//���̏コ��Ƀe�N�X�`���Asph�Aspa�A�g�D�[���e�N�X�`�����`�悵�Ă����B
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

}


