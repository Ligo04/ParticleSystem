#include"Particle.hlsli"

[maxvertexcount(2)]
void GS(
	point Vertexout gin[1],
            inout LineStream<GeoOut> lineStream)
{
    //do not draw emitter particles.
	if(gin[0].Type!=0)
    {
        //向加速度方向倾斜的直线。
        float3 po = gin[0].PosW;
        float3 p1 = gin[0].PosW + 0.07f * (float3(-1.0f, -9.8f, 0.0f));

        matrix viewProj = mul(g_View, g_Proj);
        GeoOut v0;
        v0.PosH = mul(float4(po, 1.0f), viewProj);
        v0.Tex = float2(0.0f, 0.0f);
        lineStream.Append(v0);

        GeoOut v1;
        v1.PosH = mul(float4(p1, 1.0f), viewProj);
        v1.Tex = float2(1.0f, 1.0f);
        lineStream.Append(v1);
    }
}