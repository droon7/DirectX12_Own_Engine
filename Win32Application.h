#ifndef WIN32APPLICATION_H
#define WIN32APPLICATION_H
#include"Application.h"

//�E�B���h�E�A�v���N���X
class Win32Application
{
public:
	static int WindowRun(Application* pdx12);
	static HWND GetHwnd() { return m_hwnd; }

	static void DebugOutputFormatString(const char* format, ...); //�E�B���h�E�f�o�b�O�p�֐�
	LRESULT static WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam); //�E�B���h�E�������֐�

private:
	static HWND m_hwnd;
};

#endif