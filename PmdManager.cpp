#include"PmdManager.h"



PMDData PmdLoader::getPMDData()
{
	return pmdData;
}

PMDData PmdLoader::loadPmdData(std::string strModelPath)
{

	//PMD�w�b�_�[�ǂݍ���
	char signature[3] = {};
	FILE* fp;

	auto err = fopen_s(&fp, strModelPath.c_str(), "rb");

	fread(signature, sizeof(signature), 1, fp);
	fread(&pmdData.pmdHeader, sizeof(pmdData.pmdHeader), 1, fp);

	//PMD���_���ǂݍ���
	fread(&pmdData.vertNum, sizeof(pmdData.vertNum), 1, fp);
	pmdData.vertices.resize(pmdData.vertNum * pmdvertex_size);
	fread(pmdData.vertices.data(), pmdData.vertices.size(), 1, fp);

	//PMD�C���f�b�N�X�f�[�^�ǂݍ���
	 //2�o�C�g�̃f�[�^����������unsigned short���g��
	fread(&pmdData.indicesNum, sizeof(pmdData.indicesNum), 1, fp);
	pmdData.indices.resize(pmdData.indicesNum);
	fread(pmdData.indices.data(), pmdData.indices.size() * sizeof(pmdData.indices[0]), 1, fp);


	//PMD�}�e���A���f�[�^�ǂݍ���
	fread(&pmdData.materialNum, sizeof(pmdData.materialNum), 1, fp);
	std::vector<PMDMaterial> pmdMaterialForLoad(pmdData.materialNum);

	fread(pmdMaterialForLoad.data(), pmdMaterialForLoad.size() * sizeof(PMDMaterial), 1, fp);

	pmdData.materials.resize(pmdMaterialForLoad.size());
	for (int i = 0; i < pmdMaterialForLoad.size(); ++i)
	{
		pmdData.materials[i].indicesNum = pmdMaterialForLoad[i].indicesNum;
		pmdData.materials[i].material.diffuse = pmdMaterialForLoad[i].diffuse;
		pmdData.materials[i].material.alpha = pmdMaterialForLoad[i].alpha;
		pmdData.materials[i].material.specular = pmdMaterialForLoad[i].specular;
		pmdData.materials[i].material.specularity = pmdMaterialForLoad[i].specularity;
		pmdData.materials[i].material.ambient = pmdMaterialForLoad[i].ambient;
		pmdData.materials[i].additional.toonIdx = pmdMaterialForLoad[i].toonIdx;
		pmdData.materials[i].additional.texPath = pmdMaterialForLoad[i].texFilePath;
		pmdData.materials[i].additional.edgeflag = pmdMaterialForLoad[i].edgeFlag;
	}

	fclose(fp);

	return pmdData;
}
