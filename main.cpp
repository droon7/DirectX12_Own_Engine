
#include "MyDirectX12.h"




//main�֐��@���b�Z�[�W���[�v�������Ă���
#ifdef _DEBUG
int main()
{

#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
#endif

	Application &dx12 = Application::Instance(1200,800);
	Win32App::WindowRun(&dx12);

	return 0;
}


