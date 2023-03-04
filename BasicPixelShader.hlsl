#include"BasicShaderHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
	float3 light = normalize(float3(1, -1, 1));//平行光線ベクトル
	float3 lightColor = float3(1, 1, 1);

	//ディフューズ計算
	float diffuseB = saturate(dot(-light, input.normal));
	//トゥーンシェーダー計算
	float4 toonDif = toon.Sample(smpToon, float2(0, 1.0 - diffuseB));

	//スペキュラー計算
	float3 reflectLight = normalize(reflect(light, input.normal.xyz)); //反射ベクトル作成
	float specularB = pow(saturate(dot(reflectLight, -input.ray)), specular.a);
	
	//テクスチャカラー
	float4 texColor = tex.Sample(smp, input.uv);

	//スフィアマップ用uv
	//float2 normalUV = (input.normal.xy + float2(1, -1)) * float2(0.5, -0.5);
	float2 sphereMapUV = (input.vnormal.xy + float2(1, -1)) * float2(0.5, -0.5);

	//描画する値を返す
	return  max(
		saturate(toonDif)
		* diffuse
		* texColor
		* sph.Sample(smp, sphereMapUV)
		+ saturate(spa.Sample(smp, sphereMapUV))
		+ float4(specularB * specular.rgb, 1)
		, float4(texColor * ambient/2, 1));

}