#include"pch.h"
#include "Win32Application.h"

HWND Win32Application::m_hwnd = nullptr;

Win32Application::Win32Application(UINT width, UINT height)
	: mwidth(width), mheight(height)
{
	pdx12 = Application::Instance(mwidth, mheight);
}

//デバッグ用関数
void Win32Application::DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif
}


//ウィンドウに必要な関数
LRESULT Win32Application::WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// ウィンドウが破壊されたら呼ばれる
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0); //OSに終了を伝える
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}


int Win32Application::WindowRun()
{

	//ウィンドウクラスの生成
	WNDCLASSEX w = {};

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)Win32Application::WindowProcedure; //コールバック関数の指定
	w.lpszClassName = _T(" DX 12 Sample");
	w.hInstance = GetModuleHandle(nullptr);

	RegisterClassEx(&w); //application class


	RECT wrc = { 0, 0, mwidth, mheight};//ウィンドウサイズの決定

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false); //ウィンドウサイズ補正


	//ウィンドウオブジェクトの生成
	m_hwnd = CreateWindowW(w.lpszClassName, //クラス名指定
		_T("DX12TEST"),	//タイトルバーの文字
		WS_OVERLAPPEDWINDOW, //タイトルバーとの境界線
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,//ウィンドウ幅
		wrc.bottom - wrc.top,//ウィンドウ高
		nullptr,				//親ウィンドウハンドル
		nullptr,				//メニューハンドル
		w.hInstance,			//呼び出しアプリケーションハンドル
		nullptr);			//追加パラメーター


	//DirectX12のパイプラインの初期化、リソースのロード
	pdx12->OnInit();

	pmdActors.resize(1);
	pmdActors[0].LoadPmdData("Model/初音ミクmetal.pmd");
	pmdActors[0].CreateVertexViewIndexView(pdx12);
	//pmdActors[0].PmdDraw(pdx12);

	//ウィンドウ表示
	ShowWindow(m_hwnd, SW_SHOW);


	//メッセージループの開始
	MSG msg = {};
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//DirectX12の処理
		pdx12->OnUpdate();
		pdx12->OnRender();

		//アプリケーションが終わるときmessageがWM_QUITになる
		if (msg.message == WM_QUIT)
		{
			break;
		}

	}

	//DirectX12終了時の処理
	pdx12->OnDestroy();


	//もうクラスはつかわないので登録解除する
	UnregisterClass(w.lpszClassName, w.hInstance);

	return 0;

}
