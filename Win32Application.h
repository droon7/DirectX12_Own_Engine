#ifndef WIN32APPLICATION_H
#define WIN32APPLICATION_H
#include"Application.h"

//ウィンドウアプリクラス
class Win32Application
{
public:
	static int WindowRun(Application* pdx12);
	static HWND GetHwnd() { return m_hwnd; }

	static void DebugOutputFormatString(const char* format, ...); //ウィンドウデバッグ用関数
	LRESULT static WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam); //ウィンドウ初期化関数

private:
	static HWND m_hwnd;
};

#endif