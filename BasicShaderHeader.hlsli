Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);

//���_�V�F�[�_�[�ƃs�N�Z���V�F�[�_�[�ԂŎg���\����
struct Output
{
	float4 svpos : SV_POSITION;
	float2 uv	 : TEXCOORD;
};