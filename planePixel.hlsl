#include"planeHeader.hlsli"

float4 ps(Output input) : SV_TARGET
{
	//PAL�K�i�ɂ�����O���[�X�P�[���ϊ�
	float4 color = tex.Sample(smp,input.uv);
	float Y = dot(color.rgb, float3(0.299, 0.587, 0.114));
	return float4(Y, Y, Y, 1);
}