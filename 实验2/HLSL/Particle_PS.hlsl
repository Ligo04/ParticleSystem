#include"Particle.hlsli"

float4 PS(GeoOut pin) : SV_Target
{
    return g_Tex.Sample(g_Sam, pin.Tex);
}