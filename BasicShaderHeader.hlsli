Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);

//�s��萔�o�b�t�@�[
cbuffer cbuff0 : register(b0)
{
	matrix world;
	matrix viewproj;
};

//�}�e���A���萔�o�b�t�@�[
cbuffer Material : register(b1)
{
	float4 diffuse;
	float4 specular;
	float3 ambient;
};

//���_�V�F�[�_�[�ƃs�N�Z���V�F�[�_�[�ԂŎg���\����
struct Output
{
	float4 svpos : SV_POSITION;
	float4 normal : NORMAL;
	float2 uv	 : TEXCOORD;
	
};