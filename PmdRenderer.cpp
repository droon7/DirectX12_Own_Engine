#include "pch.h"
#include "PmdRenderer.h"

HRESULT PmdRenderer::CreateGraphicsPipelineForPmd(DX12Application* app)
{
	ComPtr<ID3DBlob> vsBlob = nullptr;
	ComPtr<ID3DBlob> psBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	//���_�V�F�[�_�[�I�u�W�F�N�g�̐���
	auto result = D3DCompileFromFile(
		L"BasicVertexShader.hlsl",  //�V�F�[�_�[��
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, //�J�����g�f�B���N�g������C���N���[�h����
		"BasicVS", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, //�I�v�V�����Ƀf�o�b�O�ƍœK�����X�L�b�v
		0,
		&vsBlob, &errorBlob);

	//�G���[�������̏���
	if (FAILED(result)) {
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("�t�@�C������������܂���");
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			::OutputDebugStringA(errstr.c_str());
		}
		exit(1);
	}

	//�s�N�Z���V�F�[�_�[�I�u�W�F�N�g�̐���
	result = D3DCompileFromFile(
		L"BasicPixelShader.hlsl",  //�V�F�[�_�[��
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, //�J�����g�f�B���N�g������C���N���[�h����
		"BasicPS", "ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&psBlob, &errorBlob);

	//�G���[�������̏���
	if (FAILED(result)) {
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("�t�@�C������������܂���");
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		exit(1);
	}

	//���_���C�A�E�g�̐ݒ�
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{//���W���
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{//�@���x�N�g�����
			"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{//uv���
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
			0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{//�{�[�����
			"BONE_NO", 0, DXGI_FORMAT_R16G16_UINT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{//�{�[���E�F�C�g���
			"WEIGHT", 0, DXGI_FORMAT_R8_UINT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{//�֊s���t���O���
			"EDGE_FLG", 0, DXGI_FORMAT_R8_UINT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};


	//�p�C�v���C���X�e�[�g�̍쐬�A�ݒ�
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};
	//�V�F�[�_�[��ݒ�
	gpipeline.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength = vsBlob->GetBufferSize();
	gpipeline.PS.pShaderBytecode = psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = psBlob->GetBufferSize();
	//�T���v���}�X�N�ƃ��X�^���C�U�[�X�e�[�g��ݒ�
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//�u�����h�X�e�[�g�̐ݒ�
	gpipeline.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	//���X�^���C�U�[�̐ݒ�
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	//gpipeline.RasterizerState.MultisampleEnable = false;
	//�[�x�o�b�t�@�[�̐ݒ�
	gpipeline.DepthStencilState.DepthEnable = true;
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL; //�[�x�o�b�t�@�[�ɐ[�x�l���������݂��ۂ��̎w��
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS; //�[�x�l�̏������ق����̗p����B�@�܂苗�����߂��@�H
	gpipeline.DepthStencilState.StencilEnable = false;
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	//���̓��C�A�E�g�̐ݒ�
	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);
	//�g���C�A���O���X�g���b�v�̐ݒ�
	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	//�v���~�e�B�u�g�|���W�̐ݒ�
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//�����_�[�^�[�Q�b�g�̐ݒ�
	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	//�A���`�G�C���A�V���O�̃T���v�����̐ݒ�
	gpipeline.SampleDesc.Count = 1;
	gpipeline.SampleDesc.Quality = 0;

	//PSO�̐ݒ�̏I��

	//PSO�̍쐬

	result = app->_dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(pipelinestate.ReleaseAndGetAddressOf()));
}
