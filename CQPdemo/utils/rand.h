#pragma once

#include <random>
inline std::random_device random_rd;  // �����ڻ����������������
inline std::mt19937 random_gen(random_rd()); // �� rd() ���ֵı�׼ mersenne_twister_engine

inline int randInt(int min, int max)   // [min, max]
{
	std::uniform_int_distribution<> dis(min, max);
	return dis(random_gen);
}

inline double randReal(double min = 0.0, double max = 1.0) // [min, max)
{
	std::uniform_real_distribution<> dis(min, max);
	return dis(random_gen);
}
