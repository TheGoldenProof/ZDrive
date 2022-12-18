#pragma once

#include "ZDriveCompiler.hpp"

#include "core.hpp"

namespace ZDrive::Compiler {
	enum class TokenType {
		LPR, RPR, LBR, RBR,
		COMMA, DOT, SEMICOLON, COLON,
		PLUS, MINUS, STAR, SLASH, MOD,
		HASH, AT, SIGIL,

		EQ, EQ_EQ, NOT, NOT_EQ,
		GT, GTE, LT, LTE,

		AND, OR,

		IDENTIFIER, INT, FLOAT, VTID, PTR, LABEL, TIMESTAMP, RANK, DIFF,

		IF, IF_F, ELSE, SUB, LOOP, WHILE, WHILE_F,

		BIND,

		ERR, EOS,
	};
	using TOKEN = TokenType;

	struct Token {
		TokenType type = TOKEN::ERR;
		std::string str;
		u32 line = 0;
	};

	struct Sub {
		std::string name;
		u32 id = 0;
		i32 type = 0;
		u32 pos = 0;
		std::unordered_map<std::string, Token> bindings;
		std::unordered_map<std::string, u32> labels; // labels["smth"] returns the offset label smth refers to
		std::unordered_map<std::string, std::vector<std::pair<u32, u32>>> labelRefs; // labelRefs["smth"] returns a list of <offsets, line numbers> of all args that reference label smth
		i32 nestLevel = -1;
		i32 time = 0;
		i32 rank = -1;
		i32 diff = -1;
		std::vector<i32> code;

		inline void writeSingle(Value const& value) { Compiler::writeSingle(code, value); }
		inline void writeVal(Value const& value) { Compiler::writeVal(code, value); }
		inline void writeVar(Value const& vtid) { Compiler::writeVar(code, vtid); }
		inline void writeArg(Arg const& arg) { Compiler::writeArg(code, arg); }
		inline void writeInsHead(InsHead const& head) { Compiler::writeInsHead(code, head); }
		inline void writeIns(Ins const& ins) { Compiler::writeIns(code, ins); }
		inline void writeAll(std::vector<i32> const& src) { Compiler::writeAll(code, src); }
	};
}