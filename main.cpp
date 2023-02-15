
#include "DirectX12_Tutorial.h"




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


