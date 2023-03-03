Texture2D<float4> tex : register(t0);
Texture2D<float4> sph : register(t1);
Texture2D<float4> spa : register(t2);
Texture2D<float4> toon : register(t3);
SamplerState smp : register(s0);
SamplerState smpToon : register(s1);


////�s��萔�o�b�t�@�[
//cbuffer cbuff0 : register(b0)
//{
//	matrix world;
//	matrix view;
//	matrix projection;
//	float3 eye;
//};
//
//
////�}�e���A���萔�o�b�t�@�[
//cbuffer MaterialData : register(b1)
//{
//	float4 diffuse;
//	float4 specular;
//	float3 ambient;
//};

//�s��萔�o�b�t�@�[
cbuffer cbuff0 : register(b0)
{
	matrix view;
	matrix projection;
	float3 eye;
};

cbuffer cbuff1 : register(b1)
{
	matrix world;
};

//�}�e���A���萔�o�b�t�@�[
cbuffer MaterialData : register(b2)
{
	float4 diffuse;
	float4 specular;
	float3 ambient;
};

//���_�V�F�[�_�[�ƃs�N�Z���V�F�[�_�[�ԂŎg���\����
struct Output
{
	float4 svpos : SV_POSITION;
	float4 pos : POSITION;
	float4 normal : NORMAL0;
	float4 vnormal : NORMAL1;
	float2 uv	 : TEXCOORD;
	float3 ray :VECTOR;
};