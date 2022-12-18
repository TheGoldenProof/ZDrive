#include "ZDriveCommon.hpp"

#include <random>

#define F_ONE 0x3f800000
#define F_LARGEST_LESS_THAN_ONE 0x3f7ffff

namespace ZDrive {
	namespace {
		static std::minstd_rand rg;
	}

	void setRandSeed(u32 seed) {
		rg.seed(seed);
	}

	i32 rand() {
		return rg();
	}

	// random float in range [0, 1)
	f32 randf() {
		// generate random float in range [1, 2) for a consistent distribution
		i32 ret = F_ONE + (rg() << 8) + (rg() >> 7);
		// subtract one to put it in the right range
		return reinterpret_cast<f32&>(ret) - 1.0f;
	}
	// random float in range [-1, 1)
	f32 randf2() { return (randf() - 0.5f) * 2; }
	// random float in range [-pi, pi)
	f32 randRad() { return randf2() * static_cast<float>(M_PI); }
	// returns s as positive or negative, randomly
	i32 randSignS(i32 s) { return rg() & 1 ? s : -s; }
	// returns f as positive or negative, randomly
	f32 randSignF(f32 f) { return rg() & 1 ? f : -f; }
}