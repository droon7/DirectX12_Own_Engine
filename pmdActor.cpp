#include"PmdActor.h"

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



}

void PmdActor::GetResourceMaterialAndTexture(DX12Application* app)
{
	//�}�e���A���̐������e�N�X�`�������[�h����B�Ή�����e�N�X�`�����Ȃ����nullptr������B
	//�e�N�X�`�����ɃZ�p���[�^�[������Ε������A�K�؂Ȗ��O������
	material.textureResources.resize(pmdData.materialDatas.size());
	material.sphResources.resize(pmdData.materialDatas.size());
	material.spaResources.resize(pmdData.materialDatas.size());

	for (int i = 0; i < pmdData.materialDatas.size(); ++i)
	{
		std::string texFileName = pmdData.materialDatas[i].additional.texPath;
		std::string sphFileName = {};
		std::string spaFileName = {};

		if (texFileName.size() == 0)
		{
			material.textureResources[i] = nullptr;
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

		material.textureResources[i] = material.LoadTextureFromFile(texFilePath,app);
		material.sphResources[i] = material.LoadTextureFromFile(sphFilePath, app);
		material.spaResources[i] = material.LoadTextureFromFile(spaFilePath, app);

	}

	//�g�D�[���V�F�[�_�[�̂��߂̃J���[���b�N�A�b�v�e�[�u���̃��[�h
	material.toonResources.resize(pmdData.materialDatas.size());

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

		material.toonResources[i] = material.LoadTextureFromFile(toonFilePath, app);
	}




}