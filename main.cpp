#include"pch.h"
#include "Application.h"
#include"Win32Application.h"

//main�֐��@���b�Z�[�W���[�v�������Ă���
#ifdef _DEBUG
int main()
{

#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
#endif

	Application &dx12 = Application::Instance(1920, 1080);
	Win32Application::WindowRun(&dx12);

	return 0;
}


