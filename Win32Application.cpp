#include"pch.h"
#include "Win32Application.h"

HWND Win32Application::m_hwnd = nullptr;

//�R���X�g���N�^��DX12Application��������
Win32Application::Win32Application(UINT width, UINT height)
	: mwidth(width), mheight(height)
{
	pDX12 = DX12Application::Instance(mwidth, mheight);
}

//�f�o�b�O�p�֐�
void Win32Application::DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif
}


//�E�B���h�E�ɕK�v�Ȋ֐�
LRESULT Win32Application::WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// �E�B���h�E���j�󂳂ꂽ��Ă΂��
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0); //OS�ɏI����`����
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}




//���C�����[�v�������Ă���B�E�B���h�E�𓮂����֐��B
int Win32Application::WindowRun()
{

	//�E�B���h�E�N���X�̐���
	WNDCLASSEX w = {};

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)Win32Application::WindowProcedure; //�R�[���o�b�N�֐��̎w��
	w.lpszClassName = _T(" DX 12 Sample");
	w.hInstance = GetModuleHandle(nullptr);

	RegisterClassEx(&w); //application class


	RECT wrc = { 0, 0, mwidth, mheight};//�E�B���h�E�T�C�Y�̌���

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false); //�E�B���h�E�T�C�Y�␳


	//�E�B���h�E�I�u�W�F�N�g�̐���
	m_hwnd = CreateWindowW(w.lpszClassName, //�N���X���w��
		_T("DX12TEST"),	//�^�C�g���o�[�̕���
		WS_OVERLAPPEDWINDOW, //�^�C�g���o�[�Ƃ̋��E��
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,//�E�B���h�E��
		wrc.bottom - wrc.top,//�E�B���h�E��
		nullptr,				//�e�E�B���h�E�n���h��
		nullptr,				//���j���[�n���h��
		w.hInstance,			//�Ăяo���A�v���P�[�V�����n���h��
		nullptr);			//�ǉ��p�����[�^�[


	RunDX12();


	//�����N���X�͂���Ȃ��̂œo�^��������
	UnregisterClass(w.lpszClassName, w.hInstance);

	return 0;

}

void Win32Application::RunDX12()
{
	//DirectX12�̃p�C�v���C���̏������A���\�[�X�̃��[�h
	pDX12->OnInit(m_hwnd);

	pDX12->LoadPipeline();


	pmdRenderer.reset(new PmdRenderer(pDX12));
	std::shared_ptr<PmdActor> sharedPmdActor = std::make_shared<PmdActor>(pDX12, "Model/�����~�Nmetal.pmd");
	pmdActors.push_back(sharedPmdActor);


	//�E�B���h�E�\��
	ShowWindow(m_hwnd, SW_SHOW);


	//���b�Z�[�W���[�v�̊J�n
	MSG msg = {};
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//DirectX12�̏���
		//pDX12->OnUpdate();
		pDX12->BeginDraw();
		//pmdRenderer->SetRootsignatureAndPipelinestateAndPrimitive(pDX12);

		//PMD�p�̕`��p�C�v���C���ɍ��킹��
		pDX12->_cmdList->SetPipelineState(pmdRenderer->GetPipelinestate().Get());
		////���[�g�V�O�l�`����PMD�p�ɍ��킹��
		pDX12->_cmdList->SetGraphicsRootSignature(pmdRenderer->GetRootsignature().Get());

		pDX12->_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		pDX12->SetScene();
		pmdActors[0]->DrawPmd(pDX12);
		pDX12->EndDraw();

		//�A�v���P�[�V�������I���Ƃ�message��WM_QUIT�ɂȂ�
		if (msg.message == WM_QUIT)
		{
			break;
		}

	}

	//DirectX12�I�����̏���
	pDX12->OnDestroy();
}
