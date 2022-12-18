#include "ZDriveCompiler.hpp"

#include "scanner.hpp"

namespace ZDrive::Compiler {
	Token Scanner::ScanToken() {
		skipWhitespace();
		start = current;

		if (isAtEnd()) return makeToken(TOKEN::EOS);

		char c = advance();
		char n = peek();

		if (c == 'r' && std::isdigit(n)) return rankspec();
		if (c == 'd' && std::isdigit(n)) return diffspec();
		if (isalpha(c)) return identifier();
		if (std::isdigit(c) || (c == '-' && std::isdigit(n))) return number();
		if (c == '$' && isalnum(n)) return vartableid();

		switch (c) {
		case '(': return makeToken(TOKEN::LPR);
		case ')': return makeToken(TOKEN::RPR);
		case '{': return makeToken(TOKEN::LBR);
		case '}': return makeToken(TOKEN::RBR);
		case ',': return makeToken(TOKEN::COMMA);
		case '.': return makeToken(TOKEN::ERR); // DOT
		case '-': return makeToken(TOKEN::MINUS);
		case '+': return makeToken(TOKEN::PLUS);
		case ';': return makeToken(TOKEN::SEMICOLON);
		case ':': return makeToken(TOKEN::COLON);
		case '*': return makeToken(TOKEN::STAR);
		case '/': return makeToken(TOKEN::SLASH);
		case '%': return makeToken(TOKEN::MOD);
		case '#': return makeToken(TOKEN::HASH);
		case '@': return atSym();
		case '$': return makeToken(TOKEN::ERR); // SIGIL

		case '!': return makeToken(match('=') ? TOKEN::NOT_EQ : TOKEN::NOT);
		case '=': return makeToken(match('=') ? TOKEN::EQ_EQ : TOKEN::EQ);
		case '<': return makeToken(match('=') ? TOKEN::LTE : TOKEN::LT);
		case '>': return makeToken(match('=') ? TOKEN::GTE : TOKEN::GT);
		default: break;
		}

		return errorToken("Unexpected character");
	}



	Token Scanner::makeToken(TokenType type) const {
		return Token{ type, source.substr(start, current - start), line };
	}

	Token Scanner::errorToken(std::string msg) const {
		return Token{ TOKEN::ERR, std::move(msg), line };
	}

	Token Scanner::number() {
		if (peek() == '-') advance();
		while (std::isdigit(peek())) advance();
		if (peek() == '.' && std::isdigit(peekNext())) {
			advance();
			while (std::isdigit(peek())) advance();
			return makeToken(TOKEN::FLOAT);
		}
		return makeToken(TOKEN::INT);
	}

	Token Scanner::identifier() {
		while (isalnum(peek())) advance();
		return makeToken(identifierType());
	}

	Token Scanner::vartableid() {
		while (isalnum(peek())) advance();
		return makeToken(TOKEN::VTID);
	}

	Token Scanner::atSym() {
		char c = peek();
		if (isalpha(c)) {
			while (isalnum(peek())) advance();
			return makeToken(TOKEN::LABEL);
		} else if (c == '+' || c == '-' || std::isdigit(c)) {
			while (std::isdigit(peek())) advance();
			return makeToken(TOKEN::TIMESTAMP);
		}
		return makeToken(TOKEN::ERR);
	}

	Token Scanner::rankspec() {
		while (std::isdigit(peek())) advance();
		return makeToken(TOKEN::RANK);
	}

	Token Scanner::diffspec() {
		while (std::isdigit(peek())) advance();
		return makeToken(TOKEN::DIFF);
	}

	TokenType Scanner::identifierType() const {
		switch (source[start]) {
			/*case 'a': return checkKeyword(1, 2, "nd", TOKEN::AND);*/
		case 'b': return checkKeyword(1, 3, "ind", TOKEN::BIND);
		case 'e': return checkKeyword(1, 3, "lse", TOKEN::ELSE);
		case 'i':
		{
			i32 len = current - start;
			if (len == 2) return checkKeyword(1, 1, "f", TOKEN::IF);
			if (len == 4) return checkKeyword(1, 3, "f_f", TOKEN::IF_F);
			break;
		}
		case 'l': return checkKeyword(1, 3, "oop", TOKEN::LOOP);
			/*case 'o': return checkKeyword(1, 1, "r", TOKEN::OR);*/
		case 's': return checkKeyword(1, 2, "ub", TOKEN::SUB);
		case 'w':
		{
			i32 len = current - start;
			if (len == 5) return checkKeyword(1, 4, "hile", TOKEN::WHILE);
			if (len == 7) return checkKeyword(1, 6, "hile_f", TOKEN::WHILE_F);
		}
		}
		return TOKEN::IDENTIFIER;
	}

	TokenType Scanner::checkKeyword(u32 _start, u32 _length, std::string const& rest, TokenType type) const {
		return (
			current - start == _start + _length &&
			source.substr(start + _start, _length) == rest
			) ? type : TOKEN::IDENTIFIER;
	}

	char Scanner::peek() const {
		return source[current];
	}

	char Scanner::peekNext() const {
		return isAtEnd() ? '\0' : source[current + 1];
	}

	char Scanner::advance() {
		return source[current++];
	}

	bool Scanner::match(char expected) {
		if (isAtEnd() || source[current] != expected) return false;
		current++;
		return true;
	}

	bool Scanner::isAtEnd() const {
		return source[current] == '\0';
	}

	void Scanner::skipWhitespace() {
		for (;;) {
			char c = peek();
			switch (c) {
			case '\n': line++; __fallthrough;
			case ' ':
			case '\r':
			case '\t': advance(); break;
			case '/':
				if (peekNext() == '/') {
					while (peek() != '\n' && !isAtEnd()) advance();
				} else return;
				break;
			default: return;
			}
		}
	}
}