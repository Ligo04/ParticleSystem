#include"Particle.hlsli"

[maxvertexcount(6)]
void GS(
	point Particle gin[1],
                 inout PointStream<Particle> ptStream)
{
    gin[0].Age += gTimeStep;

    if(gin[0].Type==0)
    {
        if (gin[0].Age >= 0.002f)
        {
            for (int i = 0; i < 5;i++)
            {
                float3 vRandom = 35.0f * RandVec3((float) i / 5.0f);
                vRandom.y = 20.0f;
			
                Particle p;
                p.InitialPosW = gEmitPosW.xyz + vRandom;
                p.InitialVelW = float3(0.0f, 0.0f, 0.0f);
                p.SizeW = float2(1.0f, 1.0f);
                p.Age = 0.0f;
                p.Type = 1;
			
                ptStream.Append(p);
            }
            //重置发射时间
            gin[0].Age = 0.0f;

        }
        //总是保持发射器
        ptStream.Append(gin[0]);
    }
    else
    {
        //指定保存粒子的条件;这可能因系统而异。
        if (gin[0].Age <= 3.0f)
            ptStream.Append(gin[0]);
    }
}