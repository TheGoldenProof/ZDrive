#pragma once

namespace ZDrive {
	// <whether the operation was successful, a list of error messages, warnings, etc.>
	using ZResult = std::pair<bool, std::vector<std::string>>;

	using u8 = TGLib::u8;
	using u16 = TGLib::u16;
	using u32 = TGLib::u32;
	using u64 = TGLib::u64;

	using i8 = TGLib::i8;
	using i16 = TGLib::i16;
	using i32 = TGLib::i32;
	using i64 = TGLib::i64;

	using usize = TGLib::usize;

	using f32 = TGLib::f32;
	using f64 = TGLib::f64;
}