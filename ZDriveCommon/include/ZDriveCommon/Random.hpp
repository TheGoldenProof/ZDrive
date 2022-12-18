#pragma once

namespace ZDrive {
	void setRandSeed(u32 seed);
	// random int in range [0, INT32_MAX]
	i32 rand();
	// random float in range [0, 1)
	f32 randf();
	// random float in range [-1, 1)
	f32 randf2();
	// random float in range [-pi, pi)
	f32 randRad();
	// returns s as positive or negative, randomly
	i32 randSignS(i32 s);
	// returns f as positive or negative, randomly
	f32 randSignF(f32 f);
}