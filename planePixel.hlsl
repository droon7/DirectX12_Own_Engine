#include"planeHeader.hlsli"

//縦方向のガウスブラー
float4 VerticalBokePS(Output input) : SV_TARGET
{
	float4 color = tex.Sample(smp,input.uv);

	//縦ガウスブラー+ 法線マップによる歪み
	float w, h, levels;
	tex.GetDimensions(0, w, h, levels); //幅、高さ、ミップマップのレベル数を得る

	float dx = 1.0f / w; //1ピクセル分の幅
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

	//横ガウスブラー
	float w, h, levels;
	tex.GetDimensions(0, w, h, levels); //幅、高さ、ミップマップのレベル数を得る

	float dx = 1.0f / w; //1ピクセル分の幅
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

	//以下は試したポストエフェクト

	//何もせず描画
	//return color;


	////PAL規格におけるグレースケール変換
	//float Y = dot(color.rgb, float3(0.299, 0.587, 0.114));
	//return float4(Y, Y, Y, 1);


	////色の反転
	//return float4(1.0f - color.rgb, color.a);


	////色の4階調化 
	//return float4(color.rgb - fmod(color.rgb , 0.25f), color.a);


	////近傍テーブルの利用による単純なボカし
	//float w, h, levels;
	//tex.GetDimensions(0, w, h, levels); //幅、高さ、ミップマップのレベル数を得る

	//float dx = 1.0f / w; //1ピクセル分の幅
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


	////近傍テーブルの利用による単純なエンボス加工, 色がつくと不自然な色となるのでグレースケール
	//float w, h, levels;
	//tex.GetDimensions(0, w, h, levels); //幅、高さ、ミップマップのレベル数を得る

	//float dx = 1.0f / w; //1ピクセル分の幅
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


	////シャープネスの強調
	//float w, h, levels;
	//tex.GetDimensions(0, w, h, levels); //幅、高さ、ミップマップのレベル数を得る

	//float dx = 1.0f / w; //1ピクセル分の幅
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


	////近傍テーブルを利用した簡易的なの輪郭線の実装
	//float w, h, levels;
	//tex.GetDimensions(0, w, h, levels); //幅、高さ、ミップマップのレベル数を得る

	//float dx = 1.0f / w; //1ピクセル分の幅
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
	////return float4(color.rgb-Y, color.a); //通常の描画＋輪郭線
	//return float4(Y, color.a);			   //輪郭線のみ


}