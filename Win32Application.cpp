#include"pch.h"
#include "Win32Application.h"

HWND Win32Application::m_hwnd = nullptr;

//コンストラクタでDX12Applicationも初期化
Win32Application::Win32Application(UINT width, UINT height)
	: mwidth(width), mheight(height)
{
	pDX12 = DX12Application::Instance(mwidth, mheight);
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


//メインループが入っている。ウィンドウを動かす関数。
int Win32Application::WindowRun()
{

	//ウィンドウクラスの生成
	WNDCLASSEX w = {};

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)Win32Application::WindowProcedure; //コールバック関数の指定
	w.lpszClassName = _T(" DX 12 Sample");
	w.hInstance = GetModuleHandle(nullptr);

	RegisterClassEx(&w); //application class


	RECT wrc = { 0, 0, static_cast<LONG>(mwidth), static_cast<LONG>(mheight)};//ウィンドウサイズの決定

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

	//DirectX12を動かす。メッセージループはこの関数の中にある。
	RunDX12();


	//もうクラスはつかわないので登録解除する
	UnregisterClass(w.lpszClassName, w.hInstance);

	return 0;

}

void Win32Application::RunDX12()
{
	//DirectX12のパイプラインの初期化、リソースのロード
	pDX12->OnInit(m_hwnd);

	pDX12->LoadPipeline();


	otherRenderTarget.reset(new OtherRenderTarget(pDX12));
	pmdRenderer.reset(new PmdRenderer(pDX12));
	//std::shared_ptr<PmdActor> sharedPmdActor = std::make_shared<PmdActor>(pDX12, "Model/初音ミクmetal.pmd","motion/swing.vmd",0);
	std::shared_ptr<PmdActor> sharedPmdActor = std::make_shared<PmdActor>(pDX12, "Model/初音ミクmetal.pmd", "motion/yagokoro.vmd");
	sharedPmdActor->Move(-10, 0, 10);
	pmdActors.push_back(sharedPmdActor);
	sharedPmdActor = std::make_shared<PmdActor>(pDX12, "Model/初音ミク.pmd", "motion/swing.vmd");
	sharedPmdActor->Move(0, 0, 0);
	pmdActors.push_back(sharedPmdActor);
	sharedPmdActor = std::make_shared<PmdActor>(pDX12, "Model/カイト.pmd", "motion/motion.vmd");
	sharedPmdActor->Move(-5, 0, 5);
	pmdActors.push_back(sharedPmdActor);
	sharedPmdActor = std::make_shared<PmdActor>(pDX12, "Model/咲音メイコ.pmd", "motion/yagokoro.vmd");
	sharedPmdActor->Move(-10, 0, 0);
	pmdActors.push_back(sharedPmdActor);
	sharedPmdActor = std::make_shared<PmdActor>(pDX12, "Model/鏡音リン.pmd", "motion/yagokoro.vmd");
	sharedPmdActor->Move(10, 0, 0);
	pmdActors.push_back(sharedPmdActor);
	sharedPmdActor = std::make_shared<PmdActor>(pDX12, "Model/巡音ルカ.pmd", "motion/yagokoro.vmd");
	sharedPmdActor->Move(10, 0, 10);
	pmdActors.push_back(sharedPmdActor);

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


		//PMDモデルUpdate
		for (auto& pmd : pmdActors)
		{
			pmd->UpdatePmd();
		}


		//以下DirectX12の処理


		//まずPMDの描画をする

		//PMDモデルを光源からのデプスマップとして描画する
		pmdRenderer->PreDrawShadow(pDX12);
		pDX12->PreDrawShadowMap();
		pDX12->SetScene();
		for (auto& pmd : pmdActors)
		{
			pmd->DrawPmd(pDX12,false);
		}

		//PMDモデル描画
		pmdRenderer->PreDrawPmd(pDX12);
		otherRenderTarget->PreDrawOtherRenderTargets(pDX12);
		//シーン行列設定
		pDX12->SetScene();
		for (auto& pmd : pmdActors)
		{
			pmd->DrawPmd(pDX12,false);
		}
		pmdRenderer->PostDrawPmd(pDX12);
		otherRenderTarget->PostDrawOtherRenderTargets(pDX12);
			
		//ここまでPMDの描画


		//マルチレンダーターゲットによる描画
		//
		//otherRenderTarget->DrawOtherRenderTargetsFull(pDX12);

		//
		pDX12->SetBackBufferToRTV();
		otherRenderTarget->DrawOtherRenderTarget(pDX12);
		pDX12->EndBackBufferDraw();

		//DirectXコマンド実行
		pDX12->EndDraw();






		//アプリケーションが終わるときmessageがWM_QUITになる
		if (msg.message == WM_QUIT)
		{
			break;
		}

	}

	//DirectX12終了時の処理
	pDX12->OnDestroy();
}
