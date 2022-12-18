#pragma once

#include "ZDriveCompiler.hpp"

#include <functional>
#include <string_view>

#include "defs.hpp"
#include "scanner.hpp"

namespace ZDrive::Compiler {
	class _Compiler {
	public:
		_Compiler(LanguageDeclaration const& langDecl, std::string const& source, std::string const& entrySubName);
		std::vector<i32> operator()();

		static constexpr f32 DEFAULT_EPSILON_FACTOR = 0.0009765625f; // 1/1024 (approx. 0.1% error range)
	private:

		LangDecl const& lang;
		std::string const& src;
		std::string const& entryName;

		std::unordered_map<std::string, Token> bindings;
		std::vector<Sub> subs;
		inline std::optional<std::reference_wrapper<Sub>> currentSub() { return (subs.size() > 0 && subs.back().nestLevel > -1)? std::optional<std::reference_wrapper<Sub>>{subs.back()} : std::nullopt; }

		Token current;
		Token previous;
		bool hadError;
		bool panicMode;

		Scanner scanner;

		static u32 strToU32(std::string_view const& str);
		static Value readIntToken(Token const& token);
		std::optional<u32> extractId(Token const& token);
		std::optional<std::reference_wrapper<Sub>> findSub(std::string name);

		void errorAt(Token& token, std::string const& message);
		//void errorAtPrevious(std::string message);
		void errorAtCurrent(std::string const& message);

		bool resolveBinding(Token& token);
		bool check(TokenType type) const;
		void advance();
		bool match(TokenType type);
		void synchronize();
		void consume(TokenType type, std::string const& message);

		void topLevel();
		void bindDecl();
		void subDecl();
		void subLevel();
		void funcCall(InsDecl func);
		void subCall(Sub const& sub);
		Arg argument();
		std::optional<Arg> consumeValue();
		void label();
		void timestamp();
		void diffspec();
		void rankspec();
		void ifStatement();
		void loopStatement();
		void whileStatement();
	};
}