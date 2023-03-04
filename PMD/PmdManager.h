#ifndef PMDMANAGER_H
#define PMDMANAGER_H
//PMD�f�[�^�̊Ǘ�������
//����PMD�f�[�^�̃��[�h����N���X��pmd�\���̂ɂ��ċL�q


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
struct MaterialData
{
	unsigned int indicesNum;
	MaterialForHlsl material;
	AdditionalMaterial additional;
};

//PMDBone�f�[�^�\����
//�p�f�B���O��pack(1)�ŋl�߂�
#pragma pack(1)
struct PmdBoneData
{
	char boneName[20];		//�{�[����
	unsigned short parentNo;//�e�i���o�[
	unsigned short nextNo;  //��[�̃{�[���i���o�[
	unsigned char  Type;	//�{�[�����
	unsigned short ikBoneNo;//IK�{�[���i���o�[
	DirectX::XMFLOAT3 pos;			//�{�[������W
};
#pragma pack()

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
	std::vector<MaterialData> materialDatas;
	static constexpr size_t pmdvertex_size = 38;

	//�{�[���f�[�^
	unsigned short boneNum = 0;
	std::vector<PmdBoneData> pmdBoneDatas;

	//�t�@�C���p�X����PMD���f���f�[�^�����[�h����B
	void loadPmdData(std::string srcModelPath);
};




#endif