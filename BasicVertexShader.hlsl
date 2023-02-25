#include"BasicShaderHeader.hlsli"

Output BasicVS(
	float4 pos : POSITION,
	float4 normal : NORMAL,
	float2 uv: TEXCOORD,
	min16uint2 boneno: BONE_NO,
	min16uint weight: WEIGHT
){
	Output output;
	output.svpos = mul(mul(projection, mul(view, world)), pos);
	normal.w = 0; // •½sˆÚ“®¬•ª‚ğ–³Œø‚É‚·‚é

	float3 worldPos = mul(world, pos);
	output.ray = normalize(worldPos.xyz - eye);
	//output.ray = normalize(pos.xyz - eye);
	output.uv = uv;
	output.normal = mul(world, normal);
	output.vnormal = mul(view, output.normal);
	return output;
}