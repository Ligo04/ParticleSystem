#ifndef __RANDOM_H
#define __RANDOM_H

#include<DirectXMath.h>
#include<minwindef.h>
#include<stdlib.h>
#include<time.h>
using namespace DirectX;

class Random
{
public:
	Random();
	~Random();
	static float NextFloat(float min, float max);
	static XMFLOAT3 NextFloat(XMFLOAT3 min, XMFLOAT3 max);
	static XMFLOAT4 NextFloat(XMFLOAT4 min, XMFLOAT4 max);
	static float NextFloat();
	static int NextInt(int min, int max);
	static int NextInt(int max);
private:
	static UINT  m_seed;

};
#endif // !__RANDOM_H

