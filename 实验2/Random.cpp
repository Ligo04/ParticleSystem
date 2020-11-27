#include "Random.h"

UINT Random::m_seed = (UINT)time(0);

Random::Random()
{
}

Random::~Random()
{
}

float Random::NextFloat(float min, float max)
{
	m_seed = 214013 * m_seed + 2531011;
	return min + (m_seed >> 16)*(1.0f / 65535.0f)*(max - min);
}

XMFLOAT3 Random::NextFloat(XMFLOAT3 min, XMFLOAT3 max)
{
	XMFLOAT3 rand;
	rand.x = Random::NextFloat(min.x, max.x);
	rand.y = Random::NextFloat(min.y, max.y);
	rand.z = Random::NextFloat(min.z, max.z);
	return rand;
}

XMFLOAT4 Random::NextFloat(XMFLOAT4 min, XMFLOAT4 max)
{
	XMFLOAT4 rand;
	rand.x = Random::NextFloat(min.x, max.x);
	rand.y = Random::NextFloat(min.y, max.y);
	rand.z = Random::NextFloat(min.z, max.z);
	rand.w = Random::NextFloat(min.w, max.w);
	return rand;
}

float Random::NextFloat()
{
	return Random::NextFloat(0, 1.0f);
}

int Random::NextInt(int min, int max)
{
	srand(m_seed);
	return min + rand()%(max - min);
}

int Random::NextInt(int max)
{
	srand(m_seed);
	return  rand() % (max + 1);
}

