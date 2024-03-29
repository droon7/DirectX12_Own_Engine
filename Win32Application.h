#ifndef WIN32APPLICATION_H
#define WIN32APPLICATION_H
#include"DX12Application.h"
#include"PmdActor.h"
#include"PmdRenderer.h"

//ウィンドウアプリクラス
class Win32Application
{
private:
	static HWND m_hwnd;
	UINT mwidth;
	UINT mheight;

	DX12Application* pDX12;
	std::shared_ptr<OtherRenderTarget> otherRenderTarget;
	std::shared_ptr<PmdRenderer> pmdRenderer;
	std::vector<std::shared_ptr<PmdActor>> pmdActors;

public:
	Win32Application(UINT width, UINT height);
	int WindowRun();
	static HWND GetHwnd() { return m_hwnd; }

	static void DebugOutputFormatString(const char* format, ...); //ウィンドウデバッグ用関数
	LRESULT static WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam); //ウィンドウ初期化関数
	void RunDX12();    //メッセージループはこの中にある。
};

#endif