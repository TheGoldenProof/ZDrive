#include "ZDriveCompiler.hpp"

#include "compiler.hpp"

#include "core.hpp"


#define ASSERT_CURRENT_SUB_EXISTS_R(subVarName, returnVal) \
if (!currentSub().has_value()) { \
	errorAtCurrent(std::string(__func__) + " !currentSub().has_value()"); \
	return returnVal; \
} \
Sub& subVarName = currentSub().value();

#define ASSERT_CURRENT_SUB_EXISTS(subVarName) \
if (!currentSub().has_value()) { \
	errorAtCurrent(std::string(__func__) + " !currentSub().has_value()"); \
	return; \
} \
Sub& subVarName = currentSub().value();

namespace ZDrive::Compiler {
	using namespace Logger;

	_Compiler::_Compiler(LanguageDeclaration const& langDecl, std::string const& source, std::string const& entrySubName) :
		lang(langDecl), src(source), entryName(entrySubName),
		hadError(false), panicMode(false), scanner(source) {}

	std::vector<i32> _Compiler::operator()() {
		advance();

		while (!check(TOKEN::EOS)) {
			topLevel();
		}

		if (hadError) return {};

		const u32 headerSize = subs.size() * 3 + 2;
		u32 codeSize = headerSize;

		for (Sub& sub : subs) {
			codeSize += sub.code.size();
			
			// for each label referenced
			for (auto& pair1 : sub.labelRefs) {
				std::vector<std::pair<u32, u32>>& refs = pair1.second; // list of references to that label

				// find that label in the map of valid labels to their locations
				auto offset_it = sub.labels.find(pair1.first);
				// if its not found
				if (offset_it == sub.labels.end()) {
					// for each reference to it, print an error
					for (auto& pair2 : refs)
						Log(LL::Fatal) << "Label \"" << pair1.first << "\" (line " << pair2.second << ") not found.";
					// compilation failed
					return {};
				}

				// if it is found, get its value
				u32 offset = offset_it->second;
				// for each one, modify the arg 
				for (auto& pair2 : refs) {
					sub.code[pair2.first] = AT::CNST;
					sub.code[pair2.first+1] = offset;
				}
			}
		}

		u32 entryFuncId = static_cast<u32>(-1);
		auto entrySub_opt = findSub(entryName);
		if (!entrySub_opt) {
			Log(LL::Warn) << "Entry sub '" << entryName << " was not found. Code will compile but will not run.";
		} else {
			entryFuncId = entrySub_opt.value().get().id;
		}

		std::vector<i32> code;
		
		code.reserve(codeSize);
		code.resize(headerSize);
		u32 writePos = subs.size() * 3 + 2;
		code[0] = subs.size();
		code[1] = entryFuncId;
		for (u32 i = 0; i < subs.size(); i++) {
			Sub& sub = subs[i];
			code[i*3+2] = 0;
			code[i*3+3] = sub.code.size();
			code[i*3+4] = writePos;
			writeAll(code, sub.code);
			writePos += sub.code.size();
		}

		return code;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	u32 _Compiler::strToU32(std::string_view const& str) {
		u32 ret = 0;
		for (u32 i = 0; i < min(str.size(), sizeof(u32) / sizeof(char)); i++) {
			ret <<= 8 * sizeof(char);
			ret |= str[i];
		}
		return ret;
	}

	Value _Compiler::readIntToken(Token const& token) {
		i64 val = strtoll(token.str.c_str(), nullptr, 10);
		if (val <= INT32_MAX && val >= INT32_MIN) {
			// i forgot that return types can be implicitly converted
			return static_cast<i32>(val);
		} else {
			// i dont know how to feel about it
			return static_cast<u32>(val);
		}
	}

	std::optional<u32> _Compiler::extractId(Token const& token) {
		if (token.type == TOKEN::VTID) {
			if (std::isalpha(token.str[1])) {
				Token t{ TOKEN::IDENTIFIER, token.str.substr(1), token.line };
				resolveBinding(t);
				if (t.type == TOKEN::IDENTIFIER) {
					errorAtCurrent("Unknown identifier");
					return std::nullopt;
				}
				if (t.type == TOKEN::INT) return strtoul(t.str.c_str(), nullptr, 10);
			} else if (std::isdigit(token.str[1])) return strtoul(token.str.c_str()+1, nullptr, 10);
		}
		errorAtCurrent("Invalid VTID");
		return std::nullopt;
	}

	std::optional<std::reference_wrapper<Sub>> _Compiler::findSub(std::string name) {
		for (Sub& sub : subs) {
			if (sub.name == name) return { sub };
		} 
		return std::nullopt;
	}

	////////////////////////////////////////////////////

	void _Compiler::errorAt(Token& token, std::string const& message) {
		if (panicMode) return;
		panicMode = true;
		hadError = true;
		Log(LL::Error) << "[line " << token.line << "] Error at " << (token.type == TOKEN::EOS ? "end" : (std::string("'") + token.str + "'").c_str()) << ": " << message.c_str();
	}

	//void _Compiler::errorAtPrevious(std::string message) { errorAt(previous, message); }

	void _Compiler::errorAtCurrent(std::string const& message) { errorAt(current, message); }

	bool _Compiler::resolveBinding(Token& token) {
		i32 line = token.line;
		bool ret = false;
		bool result = true;
		while (token.type == TOKEN::IDENTIFIER && result) {

			// frick this entire body
			// in C when I could do things how i wanted, all of this was just
			// key = createTableKey(token->start, token->length);
			// result = ((info) ? tableGet(&info->bindings, key, &val) : false) || tableGet(&compiler->bindings, key, &val);
			// to me thats easier to read
			// to be fair though... i did spend another 18 lines after that just turning a string into a number

			// what this does:
			// the current sub may or may not exist depending on whether the compiler is in the global scope.
			// therefor, currentSub() returns an optional reference to a sub.
			// the lambda in .and_then takes the optional sub reference. It checks if the bindings in that sub contains a mapping from the passed in token to some other one.
			// Return an optional based on whether a binding was found. If it does, the optional is a reference to the value token, otherwise null.
			// If that returned nullopt (.or_else), then do the same for the global scope, otherwise .or_else just forwards the optional if it has a value (which would be the sub-scope binding)
			std::optional<std::reference_wrapper<Token>> val = currentSub()
				.and_then([&token](auto in) {auto val = in.get().bindings.find(token.str); return val == in.get().bindings.end() ? std::nullopt : std::optional<std::reference_wrapper<Token>>(val->second); })
				.or_else([&token, this]() {auto val = bindings.find(token.str); return val == bindings.end() ? std::nullopt : std::optional<std::reference_wrapper<Token>>(val->second); });
			// finally, if a binding was found, replace the current token we're trying to resolve with its bound value.
			// also, store whether it found a value so resolveBinding can return whether or not it was successful in resolving the binding.
			if ((result = val.has_value()) == true) {
				token = val.value();
				token.line = line;
			}
			ret |= result;
		}
		return ret;
	}

	bool _Compiler::check(TokenType type) const {
		return current.type == type;
	}

	void _Compiler::advance() {
		if (panicMode) synchronize();

		previous = current;

		for (;;) {
			current = scanner.ScanToken();
			if (current.type == TOKEN::IDENTIFIER) resolveBinding(current);
			if (current.type != TOKEN::ERR) break;

			errorAtCurrent(current.str);
		}
	}

	bool _Compiler::match(TokenType type) {
		if (!check(type)) return false;
		advance();
		return true;
	}

	void _Compiler::synchronize() {
		panicMode = false;

		while (current.type != TOKEN::EOS) {
			if (current.type == TOKEN::SEMICOLON) {
				advance();
				return;
			}
			switch (current.type) {
			case TOKEN::IF:
			case TOKEN::SUB:
			case TOKEN::LOOP:
			case TOKEN::WHILE:
			case TOKEN::BIND:
				return;
			default:;
			}

			advance();
		}
	}

	void _Compiler::consume(TokenType type, std::string const& message) {
		if (check(type)) {
			advance();
			return;
		}

		errorAtCurrent(message);
	}

	void _Compiler::topLevel() {
		switch (current.type) {
		case TOKEN::BIND:
			bindDecl();
			break;
		case TOKEN::SUB:
			subDecl();
			break;
		default: errorAtCurrent("Unexpected token"); advance();
		}
	}

	void _Compiler::bindDecl() {
		advance();
		consume(TOKEN::IDENTIFIER, "Expected identifier: binding key");
		std::string key = previous.str;
		advance();

		auto sub_opt = currentSub();
		if (sub_opt) {
			sub_opt.value().get().bindings[key] = previous;
		} else {
			bindings[key] = previous;
		}
		// frick this optionals monad functor bs
		//currentSub().transform([](auto in) {return in.get().bindings; }).value_or(bindings)[key] = previous;

		consume(TOKEN::SEMICOLON, "Expected semicolon");
	}

	void _Compiler::subDecl() {
		advance();
		consume(TOKEN::IDENTIFIER, "Expected identifier: sub name");

		Sub sub;
		sub.name = previous.str;
		sub.id = subs.size();
		consume(TOKEN::LPR, "Expected left parenthesis");

		u32 i = VTID::IN0 - 1;
		while (i <= VTID::IN7) {
			if (match(TOKEN::IDENTIFIER)) {
				i++;
				sub.bindings[previous.str] = Token{ TOKEN::INT, std::to_string(i), previous.line };
			}
			if (match(TOKEN::COMMA)) continue;
			consume(TOKEN::RPR, "Expected right parenthesis");
			break;
		}

		subs.emplace_back(std::move(sub));

		consume(TOKEN::LBR, "Expected left bracket");
		subLevel();
		consume(TOKEN::RBR, "Expected right bracket");

		// this shouldnt really be necessary but oh well.
		if (subs.back().nestLevel != -1) {
			errorAtCurrent(std::string("subDecl sub.nestLevel = ") += std::to_string(subs.back().nestLevel));
		}
	}

	void _Compiler::subLevel() {
		if (subs.size() == 0) {
			errorAtCurrent("subLevel subs.size() = 0");
			return;
		}
		Sub& sub = subs.back();

		sub.nestLevel++;
		
		while (!check(TOKEN::RBR) && !check(TOKEN::EOS)) {
			switch (current.type) {
			case TOKEN::IDENTIFIER: {
				if (auto res = lang.get(current.str); res.has_value()) {
					funcCall(res.value());
				} else if (auto toCall = findSub(current.str); toCall) { // skull emoji (inline function that checks if subs contains one with the name current.str and makes it available)
					subCall(toCall.value().get());
				} else {
					errorAtCurrent("Unexpected token: identifier");
				}
				break;
			}
			case TOKEN::SEMICOLON: advance(); break;
			case TOKEN::LABEL: label(); break;
			case TOKEN::TIMESTAMP: timestamp(); break;
			case TOKEN::DIFF: diffspec(); break;
			case TOKEN::RANK: rankspec(); break;
			case TOKEN::IF:
			case TOKEN::IF_F: ifStatement(); break;
			case TOKEN::LOOP: loopStatement(); break;
			case TOKEN::WHILE:
			case TOKEN::WHILE_F: whileStatement(); break;
			case TOKEN::BIND: bindDecl(); break;
			default:
				errorAtCurrent("Unexpected token");
				advance();
			}
		}

		if (sub.nestLevel == 0) {
			InsHead retIns{ INT32_MIN, -1, -1, INS::RET, 0 };
			sub.writeInsHead(retIns);
		}

		sub.nestLevel--;
	}

	void _Compiler::funcCall(InsDecl func) {
		ASSERT_CURRENT_SUB_EXISTS(sub);

		InsHead head{sub.time, sub.diff, sub.rank, func.code, func.argcount};
		writeInsHead(sub.code, head);

		advance();

		u32 argsWritten = 0;
		consume(TOKEN::LPR, "Expected left parenthesis");
		while (!check(TOKEN::RPR)) {
			Token arg_token = current;
			Arg arg = argument();
			if (arg.type == AT::TEMP_LABEL) {
				sub.labelRefs[arg_token.str].emplace_back(sub.code.size(), arg_token.line);
			}
			sub.writeArg(arg);
			argsWritten++;
			if (match(TOKEN::COMMA)) continue;
			break;
		}

		if (argsWritten != func.argcount)
			errorAtCurrent(std::format("{} requires {} arguments, {} found", func.identifier, func.argcount, argsWritten));

		consume(TOKEN::RPR, "Expected right parenthesis");
		consume(TOKEN::SEMICOLON, "Expected semicolon");
	}

	void _Compiler::subCall(Sub const& other) {
		ASSERT_CURRENT_SUB_EXISTS(sub);

		Ins callIns{ {sub.time, sub.diff, sub.rank, INS::CALL, 1}, {{AT::CNST, other.id}} };

		advance();
		consume(TOKEN::LPR, "Expected left parenthesis");

		u32 i = VTID::OUT0 - 1;
		while (!check(TOKEN::RPR) && i < VTID::OUT7) {
			i++;
			Token arg_token = current;
			Arg arg = argument();
			sub.writeIns(Ins(
				{sub.time, sub.diff, sub.rank, INS::SET, 2},
				{{AT::CNST, i}, arg}
			));
			if (arg.type == AT::TEMP_LABEL) {
				sub.labelRefs[arg_token.str].emplace_back(sub.code.size()-2, arg_token.line);
			}

			if (match(TOKEN::COMMA)) continue;
			break;
		}

		consume(TOKEN::RPR, "Expected right parenthesis");
		consume(TOKEN::SEMICOLON, "Expected semicolon");

		sub.writeIns(callIns);
	}

	Arg _Compiler::argument() {
		//ASSERT_CURRENT_SUB_EXISTS_R(sub, Arg{});

		advance();

		switch (previous.type) {
		case TOKEN::LABEL:
			return Arg{ AT::TEMP_LABEL, 0 };
		case TOKEN::VTID: {
			std::optional<u32> id = extractId(previous);
			if (!id) return Arg{};
			return Arg{ AT::VTREF, id.value() };
		}
		case TOKEN::FLOAT: 
			return Arg{ AT::CNST, strtof(previous.str.c_str(), nullptr) };
		case TOKEN::INT:
			return Arg{ AT::CNST, readIntToken(previous) };
		}

		errorAtCurrent("Invalid token");
		return Arg{};
	}

	std::optional<Arg> _Compiler::consumeValue() {
		//ASSERT_CURRENT_SUB_EXISTS_R(sub, std::nullopt);

		advance();

		switch (previous.type) {
		case TOKEN::VTID: {
			std::optional<Value> id = extractId(previous);
			if (!id) {
				errorAtCurrent("Invalid VTID");
				return std::nullopt;
			}
			return {{AT::VTREF, id.value()}};
		}
		case TOKEN::FLOAT:
			return {{AT::CNST, {strtof(previous.str.c_str(), nullptr)}}};
		case TOKEN::INT:
			return {{AT::CNST, readIntToken(previous)}};
		}

		errorAtCurrent("Invalid token");
		return std::nullopt;
	}

	void _Compiler::label() {
		ASSERT_CURRENT_SUB_EXISTS(sub);

		sub.labels[current.str.substr(1)] = sub.code.size();
		advance();
		consume(TOKEN::COLON, "Expected colon");
	}

	void _Compiler::timestamp() {
		ASSERT_CURRENT_SUB_EXISTS(sub);
		
		i32 time = strtol(current.str.c_str() + 1, nullptr, 10);
		char c = current.str[1];
		if (c == '+' || c == '-') {
			sub.time += time;
		} else if (std::isdigit(c)) {
			sub.time = time;
		} else {
			errorAtCurrent("Invalid timestamp");
		}
		advance();
		consume(TOKEN::COLON, "Expected colon");
	}

	void _Compiler::diffspec() {
		ASSERT_CURRENT_SUB_EXISTS(sub);

		sub.diff = strtoul(current.str.c_str() + 1, nullptr, 10);
		advance();
		consume(TOKEN::COLON, "Expected colon");
	}

	void _Compiler::rankspec() {
		ASSERT_CURRENT_SUB_EXISTS(sub);

		sub.rank = strtoul(current.str.c_str() + 1, nullptr, 10);
		advance();
		consume(TOKEN::COLON, "Expected colon");
	}

	void _Compiler::ifStatement() {
		ASSERT_CURRENT_SUB_EXISTS(sub);

		bool is_f = current.type == TOKEN::IF_F;
		advance();
		std::optional<Arg> lhs_opt = consumeValue();
		if (!lhs_opt) {
			errorAtCurrent("Invalid left comparison value in if-conditional");
			return;
		}
		Arg& lhs = lhs_opt.value();
		u32 op = INS::JMP;
		switch (current.type) {
		case TOKEN::EQ_EQ: op = INS::JMP_NEQ; break;
		case TOKEN::NOT_EQ: op = INS::JMP_EQU; break;
		case TOKEN::GT: op = INS::JMP_LTE; break;
		case TOKEN::GTE: op = INS::JMP_LT; break;
		case TOKEN::LT: op = INS::JMP_GTE; break;
		case TOKEN::LTE: op = INS::JMP_GT; break;
		default: errorAtCurrent("Expected comparison operator"); return;
		}
		op += is_f ? 1 : 0;

		advance();
		
		std::optional<Arg> rhs_opt = consumeValue();
		if (!rhs_opt) {
			errorAtCurrent("Invalid right comparison value in if-conditional");
			return;
		}
		Arg& rhs = rhs_opt.value();

		i32 time = sub.time;
		Ins jmpToElseIns{
			{time, -1, -1, op, 5},
			{
				{AT::CNST, 0},
				{AT::VTREF, VTID::CLOCK},
				lhs,
				rhs,
				{AT::CNST, max(lhs.val.f, rhs.val.f) * DEFAULT_EPSILON_FACTOR}
			}};
		sub.writeIns(jmpToElseIns);
		u32 jmpToElsePosPos = sub.code.size() - 9;

		consume(TOKEN::LBR, "Expected opening bracket");
		subLevel();
		consume(TOKEN::RBR, "Expected closing bracket");

		Ins jmpToAfterIns{
			{time, -1, -1, INS::JMP, 2},
			{
				{AT::CNST, 0},
				{AT::VTREF, VTID::CLOCK}
			}};
		sub.writeIns(jmpToElseIns);
		u32 jmpToAfterPosPos = sub.code.size() - 3;

		sub.code[jmpToElsePosPos] = sub.code.size();

		if (check(TOKEN::ELSE)) {
			advance();
			if (check(TOKEN::IF) || check(TOKEN::IF_F)) {
				ifStatement();
			} else {
				consume(TOKEN::LBR, "Expected opening bracket (or if)");
				subLevel();
				consume(TOKEN::RBR, "Expected closing bracket");
			}
		}

		sub.code[jmpToAfterPosPos] = sub.code.size();
	}

	void _Compiler::loopStatement() {
		ASSERT_CURRENT_SUB_EXISTS(sub);

		advance();
		consume(TOKEN::VTID, "Expected VTID");
		std::optional<u32> vtid_opt = extractId(previous);
		if (!vtid_opt) {
			errorAtCurrent("Invalid VTID");
			return;
		}
		u32 vtid = vtid_opt.value();
		u32 jmpBackPos = sub.code.size();
		i32 time = sub.time;

		consume(TOKEN::LBR, "Expected opening bracket");
		subLevel();
		consume(TOKEN::RBR, "Expected closing bracket");

		Ins jmpBackIns{
			{time, -1, -1, INS::LOOP, 3},
			{
				{AT::CNST, jmpBackPos},
				{AT::VTREF, VTID::CLOCK},
				{AT::CNST, vtid}
			}};
		sub.writeIns(jmpBackIns);
	}

	void _Compiler::whileStatement() {
		ASSERT_CURRENT_SUB_EXISTS(sub);

		bool is_f = current.type == TOKEN::WHILE_F;
		advance();
		std::optional<Arg> lhs_opt = consumeValue();
		if (!lhs_opt) {
			errorAtCurrent("Invalid left comparison value in if-conditional");
			return;
		}
		Arg& lhs = lhs_opt.value();
		u32 op = INS::JMP;
		switch (current.type) {
		case TOKEN::EQ_EQ: op = INS::JMP_NEQ; break;
		case TOKEN::NOT_EQ: op = INS::JMP_EQU; break;
		case TOKEN::GT: op = INS::JMP_LTE; break;
		case TOKEN::GTE: op = INS::JMP_LT; break;
		case TOKEN::LT: op = INS::JMP_GTE; break;
		case TOKEN::LTE: op = INS::JMP_GT; break;
		default: errorAtCurrent("Expected comparison operator"); return;
		}
		op += is_f ? 1 : 0;

		advance();

		std::optional<Arg> rhs_opt = consumeValue();
		if (!rhs_opt) {
			errorAtCurrent("Invalid right comparison value in if-conditional");
			return;
		}
		Arg& rhs = rhs_opt.value();

		i32 time = sub.time;
		u32 conditionPos = sub.code.size();

		Ins jmpToAfterIns{
			{time, -1, -1, op, 4},
			{
				{AT::CNST, 0},
				{AT::VTREF, VTID::CLOCK},
				lhs,
				rhs,
				{AT::CNST, max(lhs.val.f, rhs.val.f) * DEFAULT_EPSILON_FACTOR}
			}};
		sub.writeIns(jmpToAfterIns);
		u32 jmpToAfterPosPos = sub.code.size() - 9;

		consume(TOKEN::LBR, "Expected opening bracket");
		subLevel();
		consume(TOKEN::RBR, "Expected closing bracket");

		Ins jmpToCondIns{
			{time, -1, -1, INS::JMP, 2},
			{
				{AT::CNST, conditionPos},
				{AT::VTREF, VTID::CLOCK}
			}};
		sub.writeIns(jmpToCondIns);

		sub.code[jmpToAfterPosPos] = sub.code.size();
	}
}