#pragma once

#include "ZDriveCompiler.hpp"

namespace ZDrive::Compiler {
	void writeSingle(std::vector<i32>& dest, Value const& value);
	void writeVal(std::vector<i32>& dest, Value const& value);
	void writeVar(std::vector<i32>& dest, Value const& vtid);
	void writeArg(std::vector<i32>& dest, Arg const& arg);
	void writeInsHead(std::vector<i32>& dest, InsHead const& head);
	void writeIns(std::vector<i32>& dest, Ins const& ins);
	void writeAll(std::vector<i32>& dest, std::vector<i32> const& src);
}