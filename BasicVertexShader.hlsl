#include"BasicShaderHeader.hlsli"

Output BasicVS(
	float4 pos : POSITION,
	float4 normal : NORMAL,
	float2 uv: TEXCOORD,
	min16uint2 boneno: BONE_NO,
	min16uint weight: WEIGHT,
	uint instNo : SV_InstanceID
){
	Output output;

	float s_weight = weight / 100.0f;
	matrix boneMatrix = bones[boneno[0]] * s_weight + bones[boneno[1]] * (1 - s_weight);

	pos = mul(boneMatrix, pos);
	pos = mul(world, pos);
	//if (instNo == 1) {
	//	pos = mul(shadow, pos);
	//}
	//output.svpos = mul(mul(projection, view), pos);
	output.svpos = mul(lightCamera, pos);

	normal.w = 0; // ïΩçsà⁄ìÆê¨ï™Çñ≥å¯Ç…Ç∑ÇÈ

	float3 worldPos = mul(world, pos);
	output.ray = normalize(worldPos.xyz - eye);
	output.uv = uv;
	output.normal = mul(world, normal);
	output.vnormal = mul(view, output.normal);
	output.instNo = instNo;
	return output;
}