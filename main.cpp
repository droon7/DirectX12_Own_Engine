
#include "DirectX12_Tutorial.h"


// @brief �R���\�[����ʂɃt�H�[�}�b�g���������\���i�v���[�X�z���_�[)
// @param format �t�H�[�}�b�g
// @pram	�ϒ�����
// @remarks	�f�o�b�O


//main�֐��@���b�Z�[�W���[�v�������Ă���
#ifdef _DEBUG
int main()
{

#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
#endif

	Dx12 dx12(1600,1200);
	Win32App::WindowRun(&dx12);

	return 0;
}

size_t AlignmentedSize(size_t size, size_t alignment)
{
	return size + alignment - size % alignment;
}
