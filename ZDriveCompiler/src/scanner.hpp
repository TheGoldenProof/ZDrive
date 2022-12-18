#pragma once

#include "ZDriveCompiler.hpp"

#include "defs.hpp"

namespace ZDrive::Compiler {
	class Scanner {
	public:
		Scanner(std::string const& source) : source(source), start(0), current(0), line(1) {}

		Token ScanToken();
	private:
		std::string const& source;

		u32 start;
		u32 current;
		u32 line;

		inline static bool isalpha(char c) { return std::isalpha(c) || c == '_'; }
		inline static bool isalnum(char c) { return std::isalnum(c) || c == '_'; }
		Token makeToken(TokenType type) const;
		Token errorToken(std::string msg) const;
		Token number();
		Token identifier();
		Token vartableid();
		Token atSym();
		Token rankspec();
		Token diffspec();
		TokenType identifierType() const;
		TokenType checkKeyword(u32 start, u32 length, std::string const& rest, TokenType type) const;
		char peek() const;
		char peekNext() const;
		char advance();
		bool match(char expected);
		bool isAtEnd() const;
		void skipWhitespace();
	};
}