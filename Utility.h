#ifndef UTILITY_H
#define UTILITY_H
#include"pch.h"
//

//端数を切り捨てるメソッド
inline size_t AlignmentedSize(size_t size, size_t alignment)
{
	return size + alignment - size % alignment;
};

//モデルパスからファイル名を取り除き、テクスチャパスと合成するメソッド
inline std::string GetTexturePathFromModelAndTexPath(const std::string& modelPath, const char* texPath)
{
	auto folderPath = modelPath.substr(0, modelPath.rfind('/') + 1);
	return folderPath + texPath;
}

//文字列からワイド文字列を得るメソッド
std::wstring GetWideStringFromString(const std::string& str);

//ファイル名から拡張子を得るメソッド
std::string GetExtension(const std::string& path);

//テクスチャのパスをセパレーターで分離するメソッド
std::pair<std::string, std::string> SplitFileName(const std::string& path, const char splitter);

#endif