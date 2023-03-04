#include"pch.h"


//文字列からワイド文字列を取得するメソッド
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

//ファイル名から拡張子を取り出すメソッド
std::string GetExtension(const std::string& path)
{
	int idx = static_cast<int>(path.rfind('.'));
	return path.substr(idx + 1, path.length() - idx -1);
}

//セパレータでファイル名を分割してペアを返す
std::pair<std::string, std::string> SplitFileName(const std::string& path, const char splitter = '*')
{
	int idx = static_cast<int>(path.find(splitter));
	std::pair<std::string, std::string> returnPair;
	if (idx == -1) {
		returnPair.first = {};
	}
	else {
		returnPair.first = path.substr(0, idx);
	}
	returnPair.second = path.substr(idx + 1, path.length() - idx - 1);
	return returnPair;
}
