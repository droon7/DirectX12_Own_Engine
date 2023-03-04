#include"pch.h"
#include"PmdManager.h"

//�t�@�C���p�X����PMD���f���̃f�[�^��ǂݍ��ރ��\�b�h
void PmdData::loadPmdData(std::string strModelPath)
{
	FILE* fp;
	auto err = fopen_s(&fp, strModelPath.c_str(), "rb");

	//PMD�w�b�_�[�ǂݍ���
	char signature[3] = {};
	fread(signature, sizeof(signature), 1, fp);
	fread(&pmdHeader, sizeof(pmdHeader), 1, fp);

	//PMD���_���ǂݍ���
	fread(&vertNum, sizeof(vertNum), 1, fp);
	vertices.resize(vertNum * pmdvertex_size);
	fread(vertices.data(), vertices.size(), 1, fp);

	//PMD�C���f�b�N�X�f�[�^�ǂݍ���
	 //2�o�C�g�̃f�[�^����������unsigned short���g��
	fread(&indicesNum, sizeof(indicesNum), 1, fp);
	indices.resize(indicesNum);
	fread(indices.data(), indices.size() * sizeof(indices[0]), 1, fp);


	//PMD�}�e���A���f�[�^�ǂݍ���
	fread(&materialNum, sizeof(materialNum), 1, fp);
	std::vector<PMDMaterialForLoad> pmdMaterialForLoad(materialNum);

	fread(pmdMaterialForLoad.data(), pmdMaterialForLoad.size() * sizeof(PMDMaterialForLoad), 1, fp);

	materialDatas.resize(pmdMaterialForLoad.size());
	for (int i = 0; i < pmdMaterialForLoad.size(); ++i)
	{
		materialDatas[i].indicesNum = pmdMaterialForLoad[i].indicesNum;
		materialDatas[i].material.diffuse = pmdMaterialForLoad[i].diffuse;
		materialDatas[i].material.alpha = pmdMaterialForLoad[i].alpha;
		materialDatas[i].material.specular = pmdMaterialForLoad[i].specular;
		materialDatas[i].material.specularity = pmdMaterialForLoad[i].specularity;
		materialDatas[i].material.ambient = pmdMaterialForLoad[i].ambient;
		materialDatas[i].additional.toonIdx = pmdMaterialForLoad[i].toonIdx;
		materialDatas[i].additional.texPath = pmdMaterialForLoad[i].texFilePath;
		materialDatas[i].additional.edgeflag = pmdMaterialForLoad[i].edgeFlag;
	}

	//�{�[���f�[�^�ǂݍ���
	fread(&boneNum, sizeof(boneNum), 1, fp);
	pmdBones.resize(boneNum);
	fread(pmdBones.data(), sizeof(pmdBones), boneNum, fp);

	fclose(fp);

}
