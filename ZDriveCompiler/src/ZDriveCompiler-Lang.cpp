#include "ZDriveCompiler.hpp"

namespace ZDrive::Compiler {

	ZResult LanguageDeclaration::DeclareInstruction(InsDecl const& decl) {
		std::vector<std::string> msgs;
		if (ins_byCode.contains(decl.code)) {
			msgs.emplace_back(std::format("Instruction with code {} already exists. Call UndeclareInstruction first to redefine it.", decl.code));
		} else if (ins_byName.contains(decl.identifier)) {
			msgs.emplace_back(std::format("Instruction with identifier {} already exists. Call UndeclareInstruction first to redefine it.", decl.identifier));
		} else if (instructions.contains(decl)) {
			msgs.emplace_back(std::format("Bug: 'instructions' contains decl but byCode and byName don't."));
		} if (!msgs.empty()) return { false, msgs };

		instructions.emplace(decl);
		ins_byCode[decl.code] = decl;
		ins_byName[decl.identifier] = decl;
		return { true, msgs };
	}

	ZResult LanguageDeclaration::UndeclareInstruction(u32 code) {
		std::vector<std::string> msgs;
		InsDecl toErase;
		try {
			toErase = ins_byCode.at(code);
		} catch (std::out_of_range exception) {
			msgs.emplace_back(std::format("Instruction with code {} not found. If you know the identifier, try UndeclareInstruction(std::string).", code));
			return { false, msgs };
		}
		ins_byCode.erase(code);
		if (toErase.identifier.empty()) {
			msgs.emplace_back(std::format("Instruction with code {} has no identifier. This shouldn't happen.", code));
		} else {
			usize res = ins_byName.erase(toErase.identifier);
			if (!res) msgs.emplace_back(std::format("byName did not contain {}", toErase.identifier));
		}
		usize res = instructions.erase(toErase);
		if (!res) msgs.emplace_back(std::format("instructions did not contain code: {}, identifier: {}", code, toErase.identifier));
		return { true, msgs };
	}

	ZResult LanguageDeclaration::UndeclareInstruction(std::string identifier) {
		std::vector<std::string> msgs;
		InsDecl toErase;
		try {
			toErase = ins_byName.at(identifier);
		} catch (std::out_of_range exception) {
			msgs.emplace_back(std::format("Instruction with identifier {} not found. If you know the code, try UndeclareInstruction(u32).", identifier));
			return { false, msgs };
		}
		ins_byName.erase(identifier);
		usize res = ins_byCode.erase(toErase.code);
		if (!res) msgs.emplace_back(std::format("byCode did not contain {}", toErase.code));
		res = instructions.erase(toErase);
		if (!res) msgs.emplace_back(std::format("instructions did not contain code: {}, identifier: {}", toErase.code, identifier));
		return { true, msgs };
	}

	std::optional<InsDecl> LanguageDeclaration::get(u32 code) const {
		if (auto res = ins_byCode.find(code); res != ins_byCode.end())
			return { res->second };
		return std::nullopt;
	}

	std::optional<InsDecl> LanguageDeclaration::get(std::string const& name) const {
		if (auto res = ins_byName.find(name); res != ins_byName.end())
			return { res->second };
		return std::nullopt;
	}

	void LanguageDeclaration::DeclareDefaultBaseIns() {
#define di(code, argc, name) DeclareInstruction(InsDecl(static_cast<u32>(code), argc, name))

		di(INS::NOP, 0, "nop");
		di(INS::RET, 0, "ret");
		di(INS::WAIT, 1, "wait");
		di(INS::JMP, 2, "jmp");
		di(INS::LOOP, 3, "loop");
		di(INS::SET, 2, "set");
		di(INS::ISET, 2, "iset");
		di(INS::FSET, 2, "fset");
		di(INS::ISET_RAND_SIGN, 2, "iset_rand_sign");
		di(INS::FSET_RAND_SIGN, 2, "fset_rand_sign");
		di(INS::IADD, 2, "iadd");
		di(INS::ISUB, 2, "isub");
		di(INS::IMUL, 2, "imul");
		di(INS::IDIV, 2, "idiv");
		di(INS::IMOD, 2, "imod");
		di(INS::IMOD2, 2, "imod2");
		di(INS::FADD, 2, "fadd");
		di(INS::FSUB, 2, "fsub");
		di(INS::FMUL, 2, "fmul");
		di(INS::FDIV, 2, "fdiv");
		di(INS::FMOD, 2, "fmod");
		di(INS::FMOD2, 2, "fmod2");
		di(INS::ISET_ADD, 3, "iset_add");
		di(INS::ISET_SUB, 3, "iset_sub");
		di(INS::ISET_MUL, 3, "iset_mul");
		di(INS::ISET_DIV, 3, "iset_div");
		di(INS::ISET_MOD, 3, "iset_mod");
		di(INS::FSET_ADD, 3, "fset_add");
		di(INS::FSET_SUB, 3, "fset_sub");
		di(INS::FSET_MUL, 3, "fset_mul");
		di(INS::FSET_DIV, 3, "fset_div");
		di(INS::FSET_MOD, 3, "fset_mod");
		di(INS::IINC, 1, "iinc");
		di(INS::FINC, 1, "finc");
		di(INS::IDEC, 1, "idec");
		di(INS::FDEC, 1, "fdec");
		di(INS::FSET_SIN, 2, "fset_sin");
		di(INS::FSET_COS, 2, "fset_cos");
		di(INS::FSET_TAN, 2, "fset_tan");
		di(INS::FSET_ANGLE, 5, "fset_angle");
		di(INS::FINTERP, 7, "finterp");
		di(INS::NORMRAD, 1, "normRad");
		di(INS::MATHCIRCLEPOS, 4, "mathCirclePos");
		di(INS::MATHDISTANCE, 5, "mathDistance");
		di(INS::JMP_EQU, 4, "jmp_equ");
		di(INS::JMP_EQU_F, 5, "jmp_equ_f");
		di(INS::JMP_NEQ, 4, "jmp_neq");
		di(INS::JMP_NEQ_F, 5, "jmp_neq_f");
		di(INS::JMP_LT, 4, "jmp_lt");
		di(INS::JMP_LT_F, 4, "jmp_lt_f");
		di(INS::JMP_LTE, 4, "jmp_lte");
		di(INS::JMP_LTE_F, 5, "jmp_lte_f");
		di(INS::JMP_GT, 4, "jmp_gt");
		di(INS::JMP_GT_F, 4, "jmp_gt_f");
		di(INS::JMP_GTE, 4, "jmp_gte");
		di(INS::JMP_GTE_F, 5, "jmp_gte_f");
		di(INS::CALL, 1, "call");
		di(INS::YEILD, 0, "yeild");
		di(INS::PRINT, 3, "print");
		di(INS::SET_PTR, 2, "set_ptr");

#undef di
	}
}