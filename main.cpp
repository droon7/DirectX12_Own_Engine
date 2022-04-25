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

//�����I�Ƀw�b�_�[�t�@�C����
D3D_FEATURE_LEVEL levels[] =
{
	D3D_FEATURE_LEVEL_12_2,
	D3D_FEATURE_LEVEL_12_1,
	D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
};

// @brief �R���\�[����ʂɃt�H�[�}�b�g���������\���i�v���[�X�z���_�[)
// @param format �t�H�[�}�b�g
// @pram	�ϒ�����
// @remarks	�f�o�b�O

//�f�o�b�O�p�֐�
void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif
}


//�E�B���h�E�ɕK�v�Ȋ֐�
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// �E�B���h�E���j�󂳂ꂽ��Ă΂��
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0); //OS�ɏI����`����
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}


//main�֐��@���b�Z�[�W���[�v�������Ă���
#ifdef _DEBUG
int main()
{
	//�E�B���h�E�N���X�̐���
	WNDCLASSEX w = {};

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure; //�R�[���o�b�N�֐��̎w��
	w.lpszClassName = _T(" DX 12 Sample");
	w.hInstance = GetModuleHandle(nullptr);

	RegisterClassEx(&w); //application class

	int window_width = 1600;
	int window_height = 1000;
	RECT wrc = { 0, 0, window_width, window_height };//�E�B���h�E�T�C�Y�̌���

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false); //�E�B���h�E�T�C�Y�␳


	//�E�B���h�E�I�u�W�F�N�g�̐���
	HWND hwnd = CreateWindowW(w.lpszClassName, //�N���X���w��
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


	//Directx��{�I�u�W�F�N�g�̐���
	ID3D12Device* _dev = nullptr;
	IDXGIFactory6* _dxgiFactory = nullptr;
	IDXGISwapChain4* _swapchain = nullptr;

	//Directx3D�f�o�C�X������
	D3D_FEATURE_LEVEL featurelevel;
	for (auto lv : levels)
	{
		if (D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			featurelevel = lv;
			break;
		}
	}
	//DXGIFactory������
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));

	//�E�B���h�E�\��
	ShowWindow(hwnd, SW_SHOW);


	//���b�Z�[�W���[�v����
	MSG msg = {};

	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//�A�v���P�[�V�������I���Ƃ�message��WM_QUIT�ɂȂ�
		if (msg.message == WM_QUIT)
		{
			break;
		}
	}

	//�����N���X�͂���Ȃ��̂œo�^��������
	UnregisterClass(w.lpszClassName, w.hInstance);


#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#endif
	return 0;

}