#ifndef UTILITY_H
#define UTILITY_H
#include"pch.h"
//

//�[����؂�̂Ă郁�\�b�h
inline size_t AlignmentedSize(size_t size, size_t alignment)
{
	return size + alignment - size % alignment;
};

//���f���p�X����t�@�C��������菜���A�e�N�X�`���p�X�ƍ������郁�\�b�h
inline std::string GetTexturePathFromModelAndTexPath(const std::string& modelPath, const char* texPath)
{
	auto folderPath = modelPath.substr(0, modelPath.rfind('/') + 1);
	return folderPath + texPath;
}

//�����񂩂烏�C�h������𓾂郁�\�b�h
std::wstring GetWideStringFromString(const std::string& str);

//�t�@�C��������g���q�𓾂郁�\�b�h
std::string GetExtension(const std::string& path);

//�e�N�X�`���̃p�X���Z�p���[�^�[�ŕ������郁�\�b�h
std::pair<std::string, std::string> SplitFileName(const std::string& path, const char splitter);

#endif