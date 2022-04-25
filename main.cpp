#include <Windows.h>
#include <tchar.h>
#ifdef _DEBUG
#include <iostream>
#endif

#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace std;

//将来的にヘッダーファイルへ
D3D_FEATURE_LEVEL levels[] =
{
	D3D_FEATURE_LEVEL_12_2,
	D3D_FEATURE_LEVEL_12_1,
	D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
};

// @brief コンソール画面にフォーマットつき文字列を表示（プレースホルダー)
// @param format フォーマット
// @pram	可変長引数
// @remarks	デバッグ

//デバッグ用関数
void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif
}


//ウィンドウに必要な関数
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// ウィンドウが破壊されたら呼ばれる
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0); //OSに終了を伝える
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}


//main関数　メッセージループが入っている
#ifdef _DEBUG
int main()
{
	//ウィンドウクラスの生成
	WNDCLASSEX w = {};

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure; //コールバック関数の指定
	w.lpszClassName = _T(" DX 12 Sample");
	w.hInstance = GetModuleHandle(nullptr);

	RegisterClassEx(&w); //application class

	int window_width = 1600;
	int window_height = 1000;
	RECT wrc = { 0, 0, window_width, window_height };//ウィンドウサイズの決定

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false); //ウィンドウサイズ補正


	//ウィンドウオブジェクトの生成
	HWND hwnd = CreateWindowW(w.lpszClassName, //クラス名指定
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


	//Directx基本オブジェクトの宣言
	ID3D12Device* _dev = nullptr;
	IDXGIFactory6* _dxgiFactory = nullptr;
	IDXGISwapChain4* _swapchain = nullptr;
	ID3D12CommandAllocator* _cmdAllocator = nullptr;
	ID3D12GraphicsCommandList* _cmdList = nullptr;
	ID3D12CommandQueue* _cmdQueue = nullptr;

	//Directx3Dデバイス生成
	D3D_FEATURE_LEVEL featurelevel;
	for (auto lv : levels)
	{
		if (D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			featurelevel = lv;
			break;
		}
	}
	//DXGIFactory生成
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
	//コマンドアロケーターの生成
	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&_cmdAllocator));
	//コマンドリストの生成
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		_cmdAllocator, nullptr, IID_PPV_ARGS(&_cmdList));
	//コマンドキューの設定および生成
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; //タイムアウトなし
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL; //プライオリティ指定なし
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; //コマンドリストと同じタイプ
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));


	//ウィンドウ表示
	ShowWindow(hwnd, SW_SHOW);


	//メッセージループ生成
	MSG msg = {};

	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//アプリケーションが終わるときmessageがWM_QUITになる
		if (msg.message == WM_QUIT)
		{
			break;
		}
	}

	//もうクラスはつかわないので登録解除する
	UnregisterClass(w.lpszClassName, w.hInstance);


#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#endif
	return 0;

}