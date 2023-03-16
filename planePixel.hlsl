#include"planeHeader.hlsli"

float4 ps(Output input) : SV_TARGET
{
	float4 color = tex.Sample(smp,input.uv);

	//���������`��
	//return color;

	////PAL�K�i�ɂ�����O���[�X�P�[���ϊ�
	//float Y = dot(color.rgb, float3(0.299, 0.587, 0.114));
	//return float4(Y, Y, Y, 1);

	////�F�̔��]
	//return float4(1.0f - color.rgb, color.a);

	////�F��4�K���� 
	//return float4(color.rgb - fmod(color.rgb , 0.25f), color.a);

	//�ߖT�e�[�u���̗��p�ɂ��P���ȃ{�J��
	float w, h, levels;
	tex.GetDimensions(0, w, h, levels); //���A�����A�~�b�v�}�b�v�̃��x�����𓾂�

	float dx = 1.0f / w; //1�s�N�Z�����̕�
	float dy = 1.0f / h;

	int gap = 2;
	float4 ret = float4(0, 0, 0, 0);

	ret += tex.Sample(smp, input.uv + float2(-gap * dx, -gap * dy )) ;
	ret += tex.Sample(smp, input.uv + float2(0, -gap * dy));
	ret += tex.Sample(smp, input.uv + float2(gap * dx, -gap * dy));

	ret += tex.Sample(smp, input.uv + float2(-gap * dx, 0));
	ret += tex.Sample(smp, input.uv + float2(0, 0));
	ret += tex.Sample(smp, input.uv + float2(gap * dx, 0));

	ret += tex.Sample(smp, input.uv + float2(-gap * dx, gap * dy));
	ret += tex.Sample(smp, input.uv + float2(0, gap * dy));
	ret += tex.Sample(smp, input.uv + float2(gap * dy, gap * dy));

	ret = ret / 9;

	return ret;

}