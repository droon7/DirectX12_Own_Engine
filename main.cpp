#include"pch.h"
#include"DX12Application.h"
#include"Win32Application.h"

//main�֐��@���b�Z�[�W���[�v�������Ă���
#ifdef _DEBUG
int main()
{

#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
#endif

	Win32Application win32(1920, 1080);
	win32.WindowRun();

	return 0;
}


