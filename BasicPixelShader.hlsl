#include"BasicShaderHeader.hlsli"

PixelOutput BasicPS(Output input) : SV_TARGET
{
	if (input.instNo == 1)
	{
		PixelOutput po;
		po.col = float4(0, 0, 0, 1);

		po.normal.rgb = float3((input.normal.xyz + 1.0f) / 2.0f);
		po.normal.a = 1;

		po.highLum = 0.0f;

		return po;
	}

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


	//複数のレンダーターゲットに返す（現在は色と法線マップを返す）
	PixelOutput output;


	float4 ret = max(
		saturate(toonDif)
		* diffuse
		* texColor
		* sph.Sample(smp, sphereMapUV)
		* shadowWeight
		+ saturate(spa.Sample(smp, sphereMapUV))
		+ float4(specularB * specular.rgb, 1)
		, float4(texColor * ambient / 2, 1));
	output.col = ret;

	//法線が色で表現されるように加工
	output.normal.rgb = float3((input.normal.xyz + 1.0f) / 2.0f);
	output.normal.a = 1;

	float y = dot(float3 (0.299f, 0.587f, 0.114f), output.col);

	output.highLum = y > 0.99f ? y : float4(0,0,0,1);


	return output;





}