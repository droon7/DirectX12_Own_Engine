Texture2D<float4> tex : register(t0);
Texture2D<float4> sph : register(t1);
Texture2D<float4> spa : register(t2);
Texture2D<float4> toon : register(t3);
Texture2D<float4> lightDepthTex : register(t4);
SamplerState smp : register(s0);
SamplerState smpToon : register(s1);
SamplerComparisonState shadowSmp : register(s2);

//行列定数バッファー
cbuffer SceneData : register(b0)
{
	matrix view;
	matrix projection;
	matrix lightCamera;
	matrix shadow;
	float3 eye;
};

cbuffer WorldMatrix : register(b1)
{
	matrix world;
	matrix bones[256];
};

//マテリアル定数バッファー
cbuffer MaterialData : register(b2)
{
	float4 diffuse;
	float4 specular;
	float3 ambient;
};

//頂点シェーダーとピクセルシェーダー間で使う構造体
struct Output
{
	float4 svpos : SV_POSITION;
	float4 pos : POSITION;
	float4 normal : NORMAL0;
	float4 vnormal : NORMAL1;
	float2 uv	 : TEXCOORD;
	float3 ray :VECTOR;
	uint instNo : SV_InstanceID;
	float4 tpos : TPOS;
};

//複数のレンダーターゲットに対して異なる値を返す時に使う構造体
struct PixelOutput
{
	float4 col :SV_TARGET0;
	float4 normal : SV_TARGET1;
	float4 highLum : SV_TARGET2; //高輝度成分
};