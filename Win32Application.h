#ifndef WIN32APPLICATION_H
#define WIN32APPLICATION_H
#include"Application.h"
#include"PmdActor.h"

//ウィンドウアプリクラス
class Win32Application
{
public:
	Win32Application(UINT width, UINT height);
	int WindowRun();
	static HWND GetHwnd() { return m_hwnd; }

	static void DebugOutputFormatString(const char* format, ...); //ウィンドウデバッグ用関数
	LRESULT static WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam); //ウィンドウ初期化関数

private:
	static HWND m_hwnd;
	Application* pdx12;
	UINT mwidth;
	UINT mheight;

	std::vector<PmdActor> pmdActors;
};

#endif