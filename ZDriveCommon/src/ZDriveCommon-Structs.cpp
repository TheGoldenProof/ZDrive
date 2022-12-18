#include "ZDriveCommon.hpp"

namespace ZDrive {

	std::string Value::toString(u32 type) const {
		switch (type) {
		case VT::SINT:
			return std::to_string(s);
		case VT::UINT:
			return std::to_string(u);
		case VT::FLOAT:
			return std::to_string(f);
		case VT::PTR:
			return std::format("&0x{:x}", u);
		default:
		case VT::HEX:
			return std::format("0x{:x}", u);
		}
	}

	std::string Arg::toString(u32 valType) const {
		return (type == AT::VTREF) ? "$" + val.toString(VT::UINT) : val.toString(valType);
	}

	std::string InstructionHeader::toString() const {
		return std::format("time: {}, diff_mask: {}, rank_mask: {}, opcode: {}, arg_count: {}", this->time, diff_mask, rank_mask, ins, arg_count);
	}
	
	std::string Instruction::toString() const {
		std::string ret = "header: {" + header.toString() + "}, args: {";
		for (u32 i = 0; i < args.size(); i++) {
			ret += args[i].toString() + ((i == args.size() - 1) ? "}" : ", ");
		}
		return ret;
	}

#ifdef _DEBUG
	void Instruction::DebugDisassemble(const u32 offset) const {
		auto& out = Logger::Log(Logger::LL::Debug) << std::format("{:06d}   {:05d}   {:2d}   {:2d}   {:<12d}   ", offset, header.time, header.diff_mask, header.rank_mask, header.ins);
		for (Arg const& arg : args)
			out << std::format("{:12.12s} ", arg.toString());
		out << std::endl;
	}
#endif // _DEBUG

}