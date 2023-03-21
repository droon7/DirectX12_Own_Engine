#include"BasicShaderHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
	//if (input.instNo == 1)
	//{
	//	return float4(0,0,0,1);
	//}

	float3 light = normalize(float3(1, -1, 1));//���s�����x�N�g��
	float3 lightColor = float3(1, 1, 1);

	//�f�B�t���[�Y�v�Z
	float diffuseB = saturate(dot(-light, input.normal));
	//�g�D�[���V�F�[�_�[�v�Z
	float4 toonDif = toon.Sample(smpToon, float2(0, 1.0 - diffuseB));

	//�X�y�L�����[�v�Z
	float3 reflectLight = normalize(reflect(light, input.normal.xyz)); //���˃x�N�g���쐬
	float specularB = pow(saturate(dot(reflectLight, -input.ray)), specular.a);
	
	//�e�N�X�`���J���[
	float4 texColor = tex.Sample(smp, input.uv);

	//�X�t�B�A�}�b�v�puv
	//float2 normalUV = (input.normal.xy + float2(1, -1)) * float2(0.5, -0.5);
	float2 sphereMapUV = (input.vnormal.xy + float2(1, -1)) * float2(0.5, -0.5);

	//�V���h�E�}�b�v�v�Z
	//�܂����C�g���猩�����W��UV���W�ɖ߂�
	float3 posFromLightVP = input.tpos.xyz / input.tpos.w;
	float2 shadowUV = (input.tpos.xy / input.tpos.w + float2(1, -1)) * float2(0.5, -0.5);
	//���C�g���猩���[�x�Ɠ���UV���T���v��
	//float depthFromLight = lightDepthTex.Sample(smp, shadowUV);
	float depthFromLight = lightDepthTex.SampleCmp(
		shadowSmp,
		shadowUV,
		posFromLightVP.z - 0.005f
	);
	//�[�x�l���r���ĉ����ꍇ�͉e�E�F�C�g���|����B
	float shadowWeight = 1.0f;
	//if (depthFromLight < posFromLightVP.z -0.005f)
	//{
	//	shadowWeight = 0.5f;
	//}

	shadowWeight = lerp(0.5f, 1.0f, depthFromLight);

	//return shadowWeight;

	//�`�悷��l��Ԃ�
	return  max(
		saturate(toonDif)
		* diffuse
		* texColor
		* sph.Sample(smp, sphereMapUV)
		* shadowWeight
		+ saturate(spa.Sample(smp, sphereMapUV))
		+ float4(specularB * specular.rgb, 1)
		, float4(texColor * ambient/2, 1));

}