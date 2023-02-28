#ifndef WIN32APPLICATION_H
#define WIN32APPLICATION_H
#include"Application.h"
#include"PmdActor.h"

//�E�B���h�E�A�v���N���X
class Win32Application
{
public:
	Win32Application(UINT width, UINT height);
	int WindowRun();
	static HWND GetHwnd() { return m_hwnd; }

	static void DebugOutputFormatString(const char* format, ...); //�E�B���h�E�f�o�b�O�p�֐�
	LRESULT static WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam); //�E�B���h�E�������֐�

private:
	static HWND m_hwnd;
	Application* pdx12;
	UINT mwidth;
	UINT mheight;

	std::vector<PmdActor> pmdActors;
};

#endif