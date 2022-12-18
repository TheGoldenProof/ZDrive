#include "ZDriveCompiler.hpp"

#include "defs.hpp"

#include "core.hpp"

namespace ZDrive::Compiler {
	void writeSingle(std::vector<i32>& dest, Value const& value) {
		dest.push_back(value.s);
	}

	void writeVal(std::vector<i32>& dest, Value const& value) {
		dest.push_back(0);
		dest.push_back(value.s);
	}

	void writeVar(std::vector<i32>& dest, Value const& vtid) {
		dest.push_back(1);
		dest.push_back(vtid.s);
	}

	void writeArg(std::vector<i32>& dest, Arg const& arg) {
		dest.push_back(static_cast<i32>(arg.type));
		dest.push_back(arg.val.s);
	}

	void writeInsHead(std::vector<i32>& dest, InsHead const& head) {
		dest.push_back(head.time);
		dest.push_back(head.diff_mask);
		dest.push_back(head.rank_mask);
		dest.push_back(head.ins);
		dest.push_back(static_cast<i32>(head.arg_count));
	}

	void writeIns(std::vector<i32>& dest, Ins const& ins) {
		writeInsHead(dest, ins.header);
		for (Arg const& arg : ins.args) writeArg(dest, arg);
	}

	void writeAll(std::vector<i32>& dest, std::vector<i32> const& src) {
		dest.insert(dest.end(), src.begin(), src.end());
	}
}