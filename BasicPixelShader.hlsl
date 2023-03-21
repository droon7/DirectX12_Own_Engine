#include"BasicShaderHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
	//if (input.instNo == 1)
	//{
	//	return float4(0,0,0,1);
	//}

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

	//シャドウマップ計算
	//まずライトから見た座標をUV座標に戻す
	float3 posFromLightVP = input.tpos.xyz / input.tpos.w;
	float2 shadowUV = (input.tpos.xy / input.tpos.w + float2(1, -1)) * float2(0.5, -0.5);
	//ライトから見た深度と得たUVをサンプル
	//float depthFromLight = lightDepthTex.Sample(smp, shadowUV);
	float depthFromLight = lightDepthTex.SampleCmp(
		shadowSmp,
		shadowUV,
		posFromLightVP.z - 0.005f
	);
	//深度値を比較して遠い場合は影ウェイトを掛ける。
	float shadowWeight = 1.0f;
	//if (depthFromLight < posFromLightVP.z -0.005f)
	//{
	//	shadowWeight = 0.5f;
	//}

	shadowWeight = lerp(0.5f, 1.0f, depthFromLight);

	//return shadowWeight;

	//描画する値を返す
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