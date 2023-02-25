#include"MyDirectX12.h"




//�����񂩂烏�C�h��������擾���郁�\�b�h
std::wstring GetWideStringFromString(const std::string& str)
{
	auto num1 = MultiByteToWideChar(
		CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(),
		-1,
		nullptr,
		0
	);

	std::wstring wstr;
	wstr.resize(num1);

	auto num2 = MultiByteToWideChar(
		CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(),
		-1,
		&wstr[0],
		num1
	);

	assert(num1 == num2);
	return wstr;
}

//�t�@�C��������g���q�����o�����\�b�h
std::string GetExtension(const std::string& path)
{
	int idx = path.rfind('.');
	return path.substr(idx + 1, path.length() - idx -1);
}

//�Z�p���[�^�Ńt�@�C�����𕪊����ăy�A��Ԃ�
std::pair<std::string, std::string> SplitFileName(const std::string& path, const char splitter = '*')
{
	int idx = path.find(splitter);
	std::pair<std::string, std::string> returnPair;
	returnPair.first = path.substr(0, idx);
	returnPair.second = path.substr(idx + 1, path.length() - idx - 1);
	return returnPair;
}
