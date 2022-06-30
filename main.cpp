
#include "DirectX12_Tutorial.h"


// @brief コンソール画面にフォーマットつき文字列を表示（プレースホルダー)
// @param format フォーマット
// @pram	可変長引数
// @remarks	デバッグ

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 602; }

extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\"; }

//main関数　メッセージループが入っている
#ifdef _DEBUG
int main()
{

#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
#endif

	Dx12 dx12(1600,1200);
	Win32App::WindowRun(&dx12);

	return 0;
}

size_t AlignmentedSize(size_t size, size_t alignment)
{
	return size + alignment - size % alignment;
}
