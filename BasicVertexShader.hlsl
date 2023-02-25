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

	output.uv = uv;
	output.normal = mul(world, normal);
	return output;
}