#include"Particle.hlsli"

Vertexout VS(Particle vin)
{
    Vertexout vout;
	
    float t = vin.Age;
	
	// 恒定加速度方程
    vout.PosW = 0.5f * t * t * (float3(-1.0f, -9.8f, 0.0f)) + t * vin.InitialVelW + vin.InitialPosW;
	
    vout.Type = vin.Type;
	
    return vout;
}