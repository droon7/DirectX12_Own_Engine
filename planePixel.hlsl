#include"planeHeader.hlsli"

//�c�����̃K�E�X�u���[
float4 VerticalBokePS(Output input) : SV_TARGET
{
	float4 color = tex.Sample(smp,input.uv);
	return color;

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

float4 Get5x5GaussianBlur(Texture2D<float4> tex, SamplerState smp, float2 uv, float dx, float dy, float4 rect) {
	float4 ret = tex.Sample(smp, uv);

	float l1 = -dx, l2 = -2 * dx;
	float r1 = dx, r2 = 2 * dx;
	float u1 = -dy, u2 = -2 * dy;
	float d1 = dy, d2 = 2 * dy;
	l1 = max(uv.x + l1, rect.x) - uv.x;
	l2 = max(uv.x + l2, rect.x) - uv.x;
	r1 = min(uv.x + r1, rect.z - dx) - uv.x;
	r2 = min(uv.x + r2, rect.z - dx) - uv.x;

	u1 = max(uv.y + u1, rect.y) - uv.y;
	u2 = max(uv.y + u2, rect.y) - uv.y;
	d1 = min(uv.y + d1, rect.w - dy) - uv.y;
	d2 = min(uv.y + d2, rect.w - dy) - uv.y;

	return float4((
		tex.Sample(smp, uv + float2(l2, u2)).rgb
		+ tex.Sample(smp, uv + float2(l1, u2)).rgb * 4
		+ tex.Sample(smp, uv + float2(0, u2)).rgb * 6
		+ tex.Sample(smp, uv + float2(r1, u2)).rgb * 4
		+ tex.Sample(smp, uv + float2(r2, u2)).rgb

		+ tex.Sample(smp, uv + float2(l2, u1)).rgb * 4
		+ tex.Sample(smp, uv + float2(l1, u1)).rgb * 16
		+ tex.Sample(smp, uv + float2(0, u1)).rgb * 24
		+ tex.Sample(smp, uv + float2(r1, u1)).rgb * 16
		+ tex.Sample(smp, uv + float2(r2, u1)).rgb * 4

		+ tex.Sample(smp, uv + float2(l2, 0)).rgb * 6
		+ tex.Sample(smp, uv + float2(l1, 0)).rgb * 24
		+ ret.rgb * 36
		+ tex.Sample(smp, uv + float2(r1, 0)).rgb * 24
		+ tex.Sample(smp, uv + float2(r2, 0)).rgb * 6

		+ tex.Sample(smp, uv + float2(l2, d1)).rgb * 4
		+ tex.Sample(smp, uv + float2(l1, d1)).rgb * 16
		+ tex.Sample(smp, uv + float2(0, d1)).rgb * 24
		+ tex.Sample(smp, uv + float2(r1, d1)).rgb * 16
		+ tex.Sample(smp, uv + float2(r2, d1)).rgb * 4

		+ tex.Sample(smp, uv + float2(l2, d2)).rgb
		+ tex.Sample(smp, uv + float2(l1, d2)).rgb * 4
		+ tex.Sample(smp, uv + float2(0, d2)).rgb * 6
		+ tex.Sample(smp, uv + float2(r1, d2)).rgb * 4
		+ tex.Sample(smp, uv + float2(r2, d2)).rgb
		) / 256.0f, ret.a);
}


float4 BlurPS(Output input) : SV_TARGET
{
	float w,h,miplevels;
	tex.GetDimensions(0, w, h, miplevels);
	return Get5x5GaussianBlur(tex, smp, input.uv, 1.0f / w, 1.0f / h, float4(0,0,1,1));
}


float4 ps(Output input) : SV_TARGET
{

	if (input.uv.x < 0.2 && input.uv.y < 0.2)	//�[�x�}�b�v
	{
		float depth = depthTex.Sample(smp, input.uv * 5);
		depth = 1.0f - pow(depth, 500);
		return float4(depth,depth,depth, 1);
	}
	else if (input.uv.x < 0.2 && input.uv.y < 0.4) //���C�g����̐[�x
	{
		float depth = shadowMapTex.Sample(smp, (input.uv - float2(0, 0.2)) * 5);
		depth = 1 - depth;
		return float4(depth, depth, depth, 1);
	}
	else if (input.uv.x < 0.2 && input.uv.y < 0.6)
	{
		return texNormal.Sample(smp, (input.uv - float2(0, 0.4)) * 5);
	}
	else if (input.uv.x < 0.2 && input.uv.y < 0.8)
	{
		return texHighLum.Sample(smp, (input.uv - float2(0, 0.6)) * 5);
	}
	else if (input.uv.x < 0.2 )
	{
		return texShrinkHighLum.Sample(smp, (input.uv - float2(0, 0.6)) * 5);
	}

	float4 color = tex.Sample(smp,input.uv);
	//return color;
	float w, h, levels;
	tex.GetDimensions(0, w, h, levels); //���A�����A�~�b�v�}�b�v�̃��x�����𓾂�

	float dx = 1.0f / w; //1�s�N�Z�����̕�
	float dy = 1.0f / h;


	float4 bloomAccum = float4(0, 0, 0, 0);
	float2 uvSize = float2(1, 0.5);
	float2 uvOffset = float2(0, 0);

	for (int i = 0; i < 8; ++i)
	{
		bloomAccum += Get5x5GaussianBlur(texShrinkHighLum, smp, input.uv * uvSize + uvOffset, dx, dy, float4(uvOffset, uvOffset + uvSize));
		uvOffset.y += uvSize.y;
		uvSize *= 0.5f;
	}

	return color + Get5x5GaussianBlur(texHighLum, smp, input.uv, dx, dy, float4(0, 0, 1, 1)) + saturate(bloomAccum);







	////���K�E�X�u���[
	//float w, h, levels;
	//tex.GetDimensions(0, w, h, levels); //���A�����A�~�b�v�}�b�v�̃��x�����𓾂�

	//float dx = 1.0f / w; //1�s�N�Z�����̕�
	//float dy = 1.0f / h;
	//float4 ret = float4(0, 0, 0, 0);


	//ret += bkweights[0] * texHighLum.Sample(smp, input.uv);
	//for(float i = 1; i < 8; ++i)
	//{
	//	ret += bkweights[i / 4 % 2][i % 4] * texHighLum.Sample(smp, input.uv + float2(i * dx, 0));
	//	ret += bkweights[i / 4 % 2][i % 4] * texHighLum.Sample(smp, input.uv + float2(-i * dx, 0));
	//}

	//return float4(ret.rgb, color.a);

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

	
	//�@���}�b�v�ɂ��c�݃G�t�F�N�g
	//float2 nmTex = effectTex.Sample(smp, input.uv).xy;
	//nmTex = nmTex * 2.0f - 1.0f;
	//return tex.Sample(smp, input.uv + nmTex * 0.1f);
}