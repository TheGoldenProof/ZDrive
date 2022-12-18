#pragma once

namespace ZDrive::Compiler {
	struct InstructionDeclaration {
		u32 code;
		u32 argcount;
		std::string identifier;

		InstructionDeclaration() : code(0), argcount(0), identifier("uninitialized_ins") {}
		InstructionDeclaration(u32 code, u32 argcount, std::string identifier) :
			code(code), argcount(argcount), identifier(std::move(identifier)) {}

		inline bool operator==(InstructionDeclaration const& other) const {
			return ((code == other.code) && (identifier == other.identifier));
		}

		inline bool fullyEquals(InstructionDeclaration const& other) const {
			return ((code == other.code) && (argcount == other.argcount) && (identifier == other.identifier));
		}
	};
	using InsDecl = InstructionDeclaration;
}

namespace std {
	using namespace ZDrive::Compiler;
	template <>
	struct hash<ZDrive::Compiler::InstructionDeclaration> {
		size_t operator()(ZDrive::Compiler::InstructionDeclaration const& o) const {
			return (hash<ZDrive::u32>()(o.code)) ^ (hash<std::string>()(o.identifier) << 1);
		}
	};
}

namespace ZDrive::Compiler {
	class LanguageDeclaration : public LanguageBase<InsDecl> {
	protected:
		std::unordered_map<u32, InsDecl> ins_byCode;
		std::unordered_map<std::string, InsDecl> ins_byName;
	public:
		LanguageDeclaration() {}
		virtual ~LanguageDeclaration() {}

		// returns whether the operation succeeded and any errors or warnings.
		ZResult DeclareInstruction(InsDecl const&);
		ZResult UndeclareInstruction(u32 code);
		ZResult UndeclareInstruction(std::string identifier);

		std::optional<InsDecl> get(u32 code) const;
		std::optional<InsDecl> get(std::string const& name) const;

		void DeclareDefaultBaseIns();
	};
	using LangDecl = LanguageDeclaration;
}