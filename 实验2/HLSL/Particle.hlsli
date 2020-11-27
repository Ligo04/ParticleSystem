#include "LightHelper.hlsli"


Texture2D g_Tex : register(t0);
Texture1D g_Random : register(t1);
SamplerState g_Sam : register(s0);

cbuffer CBChangesEveryFrame : register(b0)
{
    float3 gEmitPosW;
    float gGameTime;
    float3 gEmitDirW;
    float gTimeStep;
    matrix g_View;
}


cbuffer CBChangesOnResize : register(b1)
{
    matrix g_Proj;
}

struct Particle
{
    float3 InitialPosW : POSITION;
    float3 InitialVelW : VELOCITY;
    float2 SizeW : SIZE;
    float Age : AGE;
    uint Type : TYPE;
};

struct Vertexout
{
    float3 PosW : POSITION;
    uint Type : TYPE;
};

struct GeoOut
{
    float4 PosH : SV_POSITION;
    float2 Tex : TEXCOORD;
};

float3 RandUnitVec3(float offeset)
{
    //游戏时间加上偏移量来从随机纹理中得到随机数
    float u = (gGameTime + offeset);

    //获得范围为[-1,1]的向量
    //float3 v = gRandomTex.SampleLevel(g_Sam, u, 0).xyz;
    float3 v = g_Random.SampleLevel(g_Sam, u, 0).xyz;

    //将得到的向量转换为单位向量
    //return normalize(v);
}

float3 RandVec3(float offeset)
{
    //游戏时间加上偏移量来从随机纹理中得到随机数
    float u = (gGameTime + offeset);
    //获得范围为[-1,1]的向量
    float3 v = g_Random.SampleLevel(g_Sam, u, 0).xyz;
    //将得到的向量转换为单位向量
    return v;
}