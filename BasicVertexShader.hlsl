#include"BasicShaderHeader.hlsli"

Output BasicVS(
	float4 pos : POSITION,
	float4 normal : NORMAL,
	float2 uv: TEXCOORD,
	min16uint2 boneno: BONE_NO,
	min16uint weight: WEIGHT
){
	Output output;

	float s_weight = weight / 100.0f;
	matrix boneMatrix = bones[boneno[0]] * s_weight + bones[boneno[1]] * (1 - s_weight);

	pos = mul(boneMatrix, pos);
	output.svpos = mul(mul(projection, mul(view, world)), pos);

	normal.w = 0; // ïΩçsà⁄ìÆê¨ï™Çñ≥å¯Ç…Ç∑ÇÈ

	float3 worldPos = mul(world, pos);
	output.ray = normalize(worldPos.xyz - eye);
	output.uv = uv;
	output.normal = mul(world, normal);
	output.vnormal = mul(view, output.normal);
	return output;
}