//頂点シェーダーとピクセルシェーダー間で使う構造体
struct Output
{
	float4 svpos : SV_POSITION;
	float2 uv	 : TEXCOORD;
};