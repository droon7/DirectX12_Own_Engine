#include"planeHeader.hlsli"

float4 ps(Output input) : SV_TARGET
{
	float4 color = tex.Sample(smp,input.uv);

	////PAL�K�i�ɂ�����O���[�X�P�[���ϊ�
	//float Y = dot(color.rgb, float3(0.299, 0.587, 0.114));
	//return float4(Y, Y, Y, 1);

	//�F�̔��]
	return float4(1.0f - color.rgb, color.a);
}