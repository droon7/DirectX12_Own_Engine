
#include "MyDirectX12.h"

HWND Win32App::m_hwnd = nullptr;

//�f�o�b�O�p�֐�
void Win32App::DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif
}


//�E�B���h�E�ɕK�v�Ȋ֐�
LRESULT Win32App::WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// �E�B���h�E���j�󂳂ꂽ��Ă΂��
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0); //OS�ɏI����`����
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}


int Win32App::WindowRun(Dx12* pdx12)
{

	//�E�B���h�E�N���X�̐���
	WNDCLASSEX w = {};

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)Win32App::WindowProcedure; //�R�[���o�b�N�֐��̎w��
	w.lpszClassName = _T(" DX 12 Sample");
	w.hInstance = GetModuleHandle(nullptr);

	RegisterClassEx(&w); //application class


	RECT wrc = { 0, 0, pdx12->window_width, pdx12->window_height };//�E�B���h�E�T�C�Y�̌���

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


	//DirectX12�̃p�C�v���C���̏������A���\�[�X�̃��[�h
	pdx12->OnInit();


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
		pdx12->OnUpdate();
		pdx12->OnRender();

		//�A�v���P�[�V�������I���Ƃ�message��WM_QUIT�ɂȂ�
		if (msg.message == WM_QUIT)
		{
			break;
		}

	}

	//DirectX12�I�����̏���
	pdx12->OnDestroy();


	//�����N���X�͂���Ȃ��̂œo�^��������
	UnregisterClass(w.lpszClassName, w.hInstance);

	return 0;

}
