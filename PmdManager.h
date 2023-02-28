#ifndef PMDMANAGER_H
#define PMDMANAGER_H

#include"pch.h"

//PMD�w�b�_�[�\����
struct PMDHeader
{
	float version;
	char model_name[20];
	char comment[256];
};

//PMD�}�e���A���\���́APMD�}�e���A���f�[�^�̓ǂݍ��݂̂��߂Ɏg��
//�p�f�B���O�����邽��#pragma pack(1)�ŃA���C�����g���l�߂�
#pragma pack(1)
struct PMDMaterialForLoad
{
	DirectX::XMFLOAT3 diffuse;   //�f�B�t���[�Y�̐F
	float alpha;				 //�f�B�t���[�Y��
	float specularity;			 //�X�y�L�����̋���
	DirectX::XMFLOAT3 specular;  //�X�y�L�����̐F
	DirectX::XMFLOAT3 ambient;   //�A���r�G���g�F
	unsigned char toonIdx;       //�g�D�[���ԍ�
	unsigned char edgeFlag;      //�}�e���A�����̗֊s���t���O
	// 2 byte padding
	unsigned int indicesNum;     //���̃}�e���A�������蓖�Ă���C���f�b�N�X��

	char texFilePath[20];        //�e�N�X�`���t�@�C���p�X�{��
};
#pragma pack()

//�V�F�[�_�[�p�}�e���A���f�[�^
struct MaterialForHlsl
{
	DirectX::XMFLOAT3 diffuse;
	float alpha;
	DirectX::XMFLOAT3 specular;
	float specularity;
	DirectX::XMFLOAT3 ambient;
};

//���̑��}�e���A���f�[�^
struct AdditionalMaterial
{
	std::string texPath;
	int toonIdx;
	bool edgeflag;
};

//�}�e���A���f�[�^���܂Ƃ߂�
struct Material
{
	unsigned int indicesNum;
	MaterialForHlsl material;
	AdditionalMaterial additional;
};


//PMD���f���̃f�[�^���܂Ƃ߂��N���X
class PmdData
{
public:
	PmdData() {};
	
	//���_���A�C���f�b�N�X���A�}�e���A�����i�[
	unsigned int vertNum = 0;
	unsigned int indicesNum = 0;
	unsigned int materialNum = 0;

	//���_�A�C���f�b�N�X�A�}�e���A���̃f�[�^
	PMDHeader pmdHeader = {};
	std::vector<unsigned char> vertices;
	std::vector<unsigned short> indices;
	std::vector<Material> materials;
	static constexpr size_t pmdvertex_size = 38;


	//�t�@�C���p�X����PMD���f���f�[�^�����[�h����B
	void loadPmdData(std::string srcModelPath);
};




#endif