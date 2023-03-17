#include"planeHeader.hlsli"

//�c�����̃K�E�X�u���[
float4 VerticalBokePS(Output input) : SV_TARGET
{
	float4 color = tex.Sample(smp,input.uv);

	//�c�K�E�X�u���[+ �@���}�b�v�ɂ��c��
	float w, h, levels;
	tex.GetDimensions(0, w, h, levels); //���A�����A�~�b�v�}�b�v�̃��x�����𓾂�

	float dx = 1.0f / w; //1�s�N�Z�����̕�
	float dy = 1.0f / h;
	float4 ret = float4(0, 0, 0, 0);


	ret += bkweights[0] * color;
	for (float i = 1; i < 8; ++i)
	{
		ret += bkweights[i / 4 % 2][i % 4] * tex.Sample(smp, input.uv + float2(0, i*dy));
		ret += bkweights[i / 4 %  2][i % 4] * tex.Sample(smp, input.uv + float2(0, -i*dy));
	}
	return float4(ret.rgb, color.a);
}

float4 ps(Output input) : SV_TARGET
{


	float4 color = tex.Sample(smp,input.uv);

	//���K�E�X�u���[
	float w, h, levels;
	tex.GetDimensions(0, w, h, levels); //���A�����A�~�b�v�}�b�v�̃��x�����𓾂�

	float dx = 1.0f / w; //1�s�N�Z�����̕�
	float dy = 1.0f / h;
	float4 ret = float4(0, 0, 0, 0);

	float2 nmTex = effectTex.Sample(smp, input.uv).xy;
	nmTex = nmTex * 2.0f - 1.0f;
	return tex.Sample(smp, input.uv + nmTex * 0.1f);

	ret += bkweights[0] * color;
	for(float i = 1; i < 8; ++i)
	{
		ret += bkweights[i / 4 % 2][i % 4] * tex.Sample(smp, input.uv + float2(i * dx, 0));
		ret += bkweights[i / 4 % 2][i % 4] * tex.Sample(smp, input.uv + float2(-i * dx, 0));
	}

	return float4(ret.rgb, color.a);

	//�ȉ��͎������|�X�g�G�t�F�N�g

	//���������`��
	//return color;


	////PAL�K�i�ɂ�����O���[�X�P�[���ϊ�
	//float Y = dot(color.rgb, float3(0.299, 0.587, 0.114));
	//return float4(Y, Y, Y, 1);


	////�F�̔��]
	//return float4(1.0f - color.rgb, color.a);


	////�F��4�K���� 
	//return float4(color.rgb - fmod(color.rgb , 0.25f), color.a);


	////�ߖT�e�[�u���̗��p�ɂ��P���ȃ{�J��
	//float w, h, levels;
	//tex.GetDimensions(0, w, h, levels); //���A�����A�~�b�v�}�b�v�̃��x�����𓾂�

	//float dx = 1.0f / w; //1�s�N�Z�����̕�
	//float dy = 1.0f / h;

	//int gap = 2;
	//float4 ret = float4(0, 0, 0, 0);

	//ret += tex.Sample(smp, input.uv + float2(-gap * dx, -gap * dy )) ;
	//ret += tex.Sample(smp, input.uv + float2(0, -gap * dy));
	//ret += tex.Sample(smp, input.uv + float2(gap * dx, -gap * dy));

	//ret += tex.Sample(smp, input.uv + float2(-gap * dx, 0));
	//ret += tex.Sample(smp, input.uv + float2(0, 0));
	//ret += tex.Sample(smp, input.uv + float2(gap * dx, 0));

	//ret += tex.Sample(smp, input.uv + float2(-gap * dx, gap * dy));
	//ret += tex.Sample(smp, input.uv + float2(0, gap * dy));
	//ret += tex.Sample(smp, input.uv + float2(gap * dy, gap * dy));

	//ret = ret / 9.0f;

	//return ret;


	////�ߖT�e�[�u���̗��p�ɂ��P���ȃG���{�X���H, �F�����ƕs���R�ȐF�ƂȂ�̂ŃO���[�X�P�[��
	//float w, h, levels;
	//tex.GetDimensions(0, w, h, levels); //���A�����A�~�b�v�}�b�v�̃��x�����𓾂�

	//float dx = 1.0f / w; //1�s�N�Z�����̕�
	//float dy = 1.0f / h;

	//int gap = 1;
	//float4 ret = float4(0, 0, 0, 0);

	//ret += tex.Sample(smp, input.uv + float2(-gap * dx, -gap * dy )) * 2;
	//ret += tex.Sample(smp, input.uv + float2(0, -gap * dy));
	//ret += tex.Sample(smp, input.uv + float2(gap * dx, -gap * dy)) * 0;

	//ret += tex.Sample(smp, input.uv + float2(-gap * dx, 0));
	//ret += tex.Sample(smp, input.uv + float2(0, 0));
	//ret += tex.Sample(smp, input.uv + float2(gap * dx, 0)) * -1;

	//ret += tex.Sample(smp, input.uv + float2(-gap * dx, gap * dy)) * 0;
	//ret += tex.Sample(smp, input.uv + float2(0, gap * dy)) * -1;
	//ret += tex.Sample(smp, input.uv + float2(gap * dy, gap * dy)) * -2;

	//float Y = dot(ret.rgb, float3(0.299, 0.587, 0.114));
	//ret =  float4(Y, Y, Y, 1);
	//return ret;


	////�V���[�v�l�X�̋���
	//float w, h, levels;
	//tex.GetDimensions(0, w, h, levels); //���A�����A�~�b�v�}�b�v�̃��x�����𓾂�

	//float dx = 1.0f / w; //1�s�N�Z�����̕�
	//float dy = 1.0f / h;

	//int gap = 1;
	//float4 ret = float4(0, 0, 0, 0);

	//ret += tex.Sample(smp, input.uv + float2(-gap * dx, -gap * dy )) * 0;
	//ret += tex.Sample(smp, input.uv + float2(0, -gap * dy)) * -1 ;
	//ret += tex.Sample(smp, input.uv + float2(gap * dx, -gap * dy)) * 0;

	//ret += tex.Sample(smp, input.uv + float2(-gap * dx, 0)) * -1;
	//ret += tex.Sample(smp, input.uv + float2(0, 0)) * 5;
	//ret += tex.Sample(smp, input.uv + float2(gap * dx, 0)) * -1;

	//ret += tex.Sample(smp, input.uv + float2(-gap * dx, gap * dy)) * 0;
	//ret += tex.Sample(smp, input.uv + float2(0, gap * dy)) * -1;
	//ret += tex.Sample(smp, input.uv + float2(gap * dy, gap * dy)) * 0;
	//return ret;


	////�ߖT�e�[�u���𗘗p�����ȈՓI�Ȃ̗֊s���̎���
	//float w, h, levels;
	//tex.GetDimensions(0, w, h, levels); //���A�����A�~�b�v�}�b�v�̃��x�����𓾂�

	//float dx = 1.0f / w; //1�s�N�Z�����̕�
	//float dy = 1.0f / h;

	//int gap = 2;
	//float4 ret = float4(0, 0, 0, 0);

	//ret += tex.Sample(smp, input.uv + float2(-gap * dx, -gap * dy)) * 0;
	//ret += tex.Sample(smp, input.uv + float2(0, -gap * dy)) * -1;
	//ret += tex.Sample(smp, input.uv + float2(gap * dx, -gap * dy)) * 0;

	//ret += tex.Sample(smp, input.uv + float2(-gap * dx, 0)) * -1;
	//ret += tex.Sample(smp, input.uv + float2(0, 0)) * 4;
	//ret += tex.Sample(smp, input.uv + float2(gap * dx, 0)) * -1;

	//ret += tex.Sample(smp, input.uv + float2(-gap * dx, gap * dy)) * 0;
	//ret += tex.Sample(smp, input.uv + float2(0, gap * dy)) * -1;
	//ret += tex.Sample(smp, input.uv + float2(gap * dy, gap * dy)) * 0;

	//float3 Y = dot(ret.rgb, float3(0.299, 0.587, 0.114));
	//Y = pow(1.0f - Y, 20.0f);
	//Y = step(0.2f, Y);
	//Y = 1.0f - Y;
	////return float4(color.rgb-Y, color.a); //�ʏ�̕`��{�֊s��
	//return float4(Y, color.a);			   //�֊s���̂�


}