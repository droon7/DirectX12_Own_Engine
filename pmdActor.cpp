#include"PmdActor.h"

void PmdActor::LoadPmdData(std::string ModelName)
{
	pmdData.loadPmdData(ModelName);
}

void PmdActor::CreateVertexViewIndexView(Application* app)
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