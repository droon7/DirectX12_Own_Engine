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


float4 ShadowVS(
	float4 pos : POSITION,
	float4 normal : NORMAL,
	float2 uv : TEXCOORD,
	min16uint2 boneno : BONE_NO,
	min16uint weight : WEIGHT,
	uint instNo : SV_InstanceID
) : SV_POSITION 
{

	float s_weight = weight / 100.0f;
	matrix boneMatrix = bones[boneno[0]] * s_weight + bones[boneno[1]] * (1 - s_weight);

	pos = mul(boneMatrix, pos);
	pos = mul(world, pos);

	return mul(lightCamera, pos);
}