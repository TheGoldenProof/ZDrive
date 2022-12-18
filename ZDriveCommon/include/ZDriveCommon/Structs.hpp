#pragma once

namespace ZDrive {
	// bits are layed out like so:
	// bbbbbbbb bbbbbbbb bbbbvvvv vvvvvvvv
	// b: block id
	// v: vartable id
	struct ValPtr {
		unsigned v : 12;
		unsigned b : 20;

		ValPtr() : v(0), b(0) {}
		ValPtr(u32 blockId, u32 varId) : v(varId), b(blockId) {}
		ValPtr(u32 other) : v(other & 0xfff), b(other >> 12) {} // dangerous
		ValPtr(const ValPtr& other) : v(other.v), b(other.b) {}
		constexpr operator u32() const { return (b << 12) + v; }
	};

	union Value {
		i32 s;
		u32 u;
		f32 f;
		ValPtr p;

		Value() : s(0) {}
		Value(u32 _u) : u(_u) {}
		Value(i32 _s) : s(_s) {}
		Value(f32 _f) : f(_f) {}
		Value(ValPtr _p) : p(_p) {}
		Value(Value const& other) : u(other.u) {}

		std::string toString(u32 type = VT::HEX) const;

		inline constexpr operator i32() const { return s; }
		inline constexpr operator u32() const { return u; }
		inline constexpr operator f32() const { return f; }
		inline constexpr bool operator==(Value const& other) const { return u == other.u; }
		inline constexpr bool operator!=(Value const& other) const { return u != other.u; }
	};

	struct Arg {
		i32 type;
		Value val;

		Arg() : type(AT::CNST), val(0) {}
		Arg(i32 type, Value val) : type(type), val(val) {}
		std::string toString(u32 valType = VT::HEX) const;
	};

	struct InstructionHeader {
		i32 time;
		i32 diff_mask;
		i32 rank_mask;
		u32 ins;
		u32 arg_count;

		InstructionHeader() : time(0), diff_mask(0), rank_mask(0), ins(0), arg_count(0) {}
		InstructionHeader(i32 time, i32 diff_mask, i32 rank_mask, u32 ins, u32 arg_count) : 
			time(time), diff_mask(diff_mask), rank_mask(rank_mask), ins(ins), arg_count(arg_count) {}

		std::string toString() const;
	};
	using InsHead = InstructionHeader;

	struct Instruction {
		InstructionHeader header;
		std::vector<Arg> args;

		Instruction() {}
		Instruction(InstructionHeader header, std::vector<Arg> args) : header(header), args(std::move(args)) {}

		template <std::input_iterator It>
		Instruction(It iter) {
			header = { *iter++, *iter++, *iter++, static_cast<u32>(*iter++), static_cast<u32>(*iter++) };
			args.reserve(header.arg_count);
			for (u32 i = 0; i < header.arg_count; i++) {
				i32 type = *iter++;
				Value val = *iter++;
				args.emplace_back(type, val);
			}
		}

		constexpr usize size() const { return sizeof(InstructionHeader)/sizeof(u32) + args.size() * 2; }
		std::string toString() const;
#ifdef _DEBUG
		void DebugDisassemble(u32 offset = 0) const;
#endif // _DEBUG

	};
	using Ins = Instruction;

	constexpr const u32 INS_TIMESTAMP = 0;
	constexpr const u32 INS_DIFFMASK = 1;
	constexpr const u32 INS_RANKMASK = 2;
	constexpr const u32 INS_CODE = 3;
	constexpr const u32 INS_ARGCOUNT = 4;
	constexpr const u32 INS_HEADER_SIZE = (sizeof(InsHead) / sizeof(i32));
}