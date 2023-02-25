#include"BasicShaderHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
	float3 light = normalize(float3(1, -1, 1));//平行光線ベクトル
	float3 lightColor = float3(1, 1, 1);

	//ディフューズ計算
	float diffuseB = dot(-light, input.normal);

	//スペキュラー計算
	float3 reflectLight = normalize(reflect(light, input.normal.xyz)); //反射ベクトル作成
	float specularB = pow(saturate(dot(reflectLight, -input.ray)), specular.a);
	
	//テクスチャカラー
	float4 texColor = tex.Sample(smp, input.uv);

	//スフィアマップ用uv
	//float2 normalUV = (input.normal.xy + float2(1, -1)) * float2(0.5, -0.5);
	float2 sphereMapUV = (input.vnormal.xy + float2(1, -1)) * float2(0.5, -0.5);

	//描画する値を返す
	return  max(diffuseB
		* diffuse
		* texColor
		* sph.Sample(smp, sphereMapUV)
		+ spa.Sample(smp, sphereMapUV)
		+ float4(specularB * specular.rgb, 1)
		//, float4(texColor * ambient, 1));
		,0);

	//return float4(specularB * specular.rgb, 1);
}