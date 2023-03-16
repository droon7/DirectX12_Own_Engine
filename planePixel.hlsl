#include"planeHeader.hlsli"

float4 ps(Output input) : SV_TARGET
{
	float4 color = tex.Sample(smp,input.uv);

	//何もせず描画
	//return color;

	////PAL規格におけるグレースケール変換
	//float Y = dot(color.rgb, float3(0.299, 0.587, 0.114));
	//return float4(Y, Y, Y, 1);

	////色の反転
	//return float4(1.0f - color.rgb, color.a);

	////色の4階調化 
	//return float4(color.rgb - fmod(color.rgb , 0.25f), color.a);

	//近傍テーブルの利用による単純なボカし
	float w, h, levels;
	tex.GetDimensions(0, w, h, levels); //幅、高さ、ミップマップのレベル数を得る

	float dx = 1.0f / w; //1ピクセル分の幅
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