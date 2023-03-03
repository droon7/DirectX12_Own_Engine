#include "pch.h"
#include "PmdRenderer.h"

PmdRenderer::PmdRenderer(DX12Application* app)
{
	CreateGraphicsPipelineForPmd(app);
	CreateRootSignature(app);

}

ComPtr<ID3D12RootSignature> PmdRenderer::GetRootsignature()
{
	return rootsignature;
}

ComPtr<ID3D12PipelineState> PmdRenderer::GetPipelinestate()
{
	return pipelinestate;
}

void PmdRenderer::SetRootsignatureAndPipelinestateAndPrimitive(DX12Application* app)
{
	app->_cmdList->SetComputeRootSignature(rootsignature.Get());
	app->_cmdList->SetPipelineState(pipelinestate.Get());
	app->_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}

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
	return result;
}

HRESULT PmdRenderer::CreateRootSignature(DX12Application* app)
{
	//���[�g�V�O�l�`���ɐݒ肷�郋�[�g�p�����[�^�y�уf�B�X�N���v�^�e�[�u���A�f�B�X�N���v�^�����W�̐ݒ�
	CD3DX12_DESCRIPTOR_RANGE descTblRange[4] = {};
	descTblRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);//CBV b0 viewProjectMatrix
	descTblRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);//CBV b1 worldTransformMatrix
	descTblRange[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);//CBV b2 Material
	descTblRange[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);//SRV t0 - t3 Textures

	//���[�g�p�����[�^�[�̐ݒ�
	CD3DX12_ROOT_PARAMETER rootparam[3] = {};

	rootparam[0].InitAsDescriptorTable(1, &descTblRange[0]);// view projection matrix;
	rootparam[1].InitAsDescriptorTable(1, &descTblRange[1]);// worldTransformMatrix;
	rootparam[2].InitAsDescriptorTable(2, &descTblRange[2]);// material;


	//���[�g�V�O�l�`���ɐݒ肷��T���v���[�̐ݒ�
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc[2] = {};

	samplerDesc[0].Init(0);
	samplerDesc[1].Init(1, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	//���[�g�V�O�l�`���̐ݒ�A����
	//���[�g�V�O�l�`���f�B�X�N���v�^�̐ݒ�
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//���[�g�p�����[�^�[��ݒ�
	rootSignatureDesc.pParameters = rootparam;
	rootSignatureDesc.NumParameters = 3;
	//�T���v���[��ݒ�
	rootSignatureDesc.pStaticSamplers = samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 2;

	//���[�g�V�O�l�`����Blob�̍쐬
	ComPtr<ID3DBlob> rootSigBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	auto result = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob,
		&errorBlob);

	//���[�g�V�O�l�`���I�u�W�F�N�g�̍쐬
	result = app->_dev->CreateRootSignature(
		0,
		rootSigBlob->GetBufferPointer(),
		rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(rootsignature.ReleaseAndGetAddressOf()));


	return result;
}