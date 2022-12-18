#include "ZDriveVM.hpp"

namespace ZDrive::VM {

	std::string Routine::toString() {
		return std::format("{{instance: {}, sub: {}, type: {}}}", instanceId, subId, GetTypeID());
	}

	std::string Routine::toStringShortened() {
		return std::format("{{{}, {}, {}}}", instanceId, subId, GetTypeID());
	}

	i32 Routine::SetVar(u32 id, Value val) {
		ValPtr vptr = id;
		return vptr.b ? vm.SetVarByPtr(vptr, val, *this) : IHasVarTable::SetVar(vptr.v, val);
	}

	std::optional<Value> Routine::GetVar(u32 id) {
		ValPtr vptr = id;
		return vptr.b ? vm.GetVarByPtr(vptr, *this) : IHasVarTable::GetVar(vptr.v);
	}

	std::optional<std::reference_wrapper<Value>> Routine::GetVarRef(u32 id) {
		ValPtr vptr = id;
		return vptr.b ? vm.GetVarRefByPtr(vptr, *this) : IHasVarTable::GetVarRef(vptr.v);
	}

	std::optional<ProcessedInstruction> Routine::ProcessInstruction(Instruction const& ins) {
		PrxIns ret;
		ret.opcode = ins.header.ins;
		for (Arg const& arg : ins.args) {
			std::optional<Value> val = ResolveArg(arg);
			if (val) {
				ret.args.push_back(val.value());
			} else {
				return std::nullopt;
			}
		}
		return ret;
	}

	std::optional<Value> Routine::ResolveArg(Arg const& arg) {
		switch (arg.type) {
		case AT::CNST: return arg.val;
		case AT::VTREF: {
			auto ret = GetVar(arg.val);
			if (!ret) Logger::Log(Logger::LL::Error) << "Could not resolve argument " << arg.toString(AT::VTREF) << ": variable not found." << std::endl;
			return ret;
		}
		case AT::TEMP_LABEL:
			Logger::Log(Logger::LL::Error) << "TEMP_LABEL ArgType in ResolveArg" << std::endl;
			return std::nullopt;
		default:
			Logger::Log(Logger::LL::Error) << "Unknown ArgType: " << arg.type << std::endl;
			return std::nullopt;
		}
	}

	bool Routine::Update() {
		bool handleSuccess = true;
		Ins cur_ins;
		VarTableVar& clock = vt[VTID::CLOCK];

		while (cur_ins = Ins(code.begin() + ptr), clock.val.s >= cur_ins.header.time) {
			nextPtr = ptr + static_cast<u32>(cur_ins.size());
			bool shouldReturn = false;
			std::optional<PrxIns> prx_ins = ProcessInstruction(cur_ins);
			if (!prx_ins) {
				Logger::Log(Logger::LL::Error) << "Skipping instruction that failed to process: " << std::endl;
				Logger::Log(Logger::LL::Error) << cur_ins.toString() << std::endl;
			} else {
				i32 diff_mask = cur_ins.header.diff_mask;
				i32 rank_mask = cur_ins.header.rank_mask;
				std::optional<Value> diffopt = vm.GetVar(VTID::DIFF);
				std::optional<Value> rankopt = vm.GetVar(VTID::RANK);
				if ((!diffopt || diff_mask & diffopt.value().s) &&
					(!rankopt || rank_mask & rankopt.value().s)) {
					HandleResult result = Handle(prx_ins.value());
					handleSuccess |= result.success;
					shouldReturn = result.shouldReturn;
					deleteMe = result.deleteMe;

					if (!result.success) {
						Logger::Log(Logger::LL::Error) << "Error while handling instruction: " << std::endl;
						Logger::Log(Logger::LL::Error) << cur_ins.toString() << std::endl;
						if (result.deleteMe) {
							Logger::Log(Logger::LL::Error) << "Routine {instance: " << instanceId << ", sub: " << subId << ", type: " << GetTypeID() << "} had a fatal error and will be terminated." << std::endl;
						}
					}
				}
			}
			ptr = nextPtr;
			if (shouldReturn) break;
		}
		vt[VTID::TIME].val.u++;
		clock.val.s++;

		return handleSuccess;
	}

	i32 Routine::try_set(u32 id, Value const& value) {
		i32 result = SetVar(id, value);
		if (result == 1) {
			Logger::Log(Logger::LL::Error) << "Could not find variable " << id << std::endl;
		} else if (result == 2) {
			Logger::Log(Logger::LL::Error) << "Variable " << id << " is read only." << std::endl;
		}
		return result;
	}

	i32 Routine::unary_op(u32 id, Value a, std::function<Value(Value)> func) {
		return try_set(id, func(a));
	}

	i32 Routine::self_unary_op(u32 a_id, std::function<Value(Value)> func) {
		std::optional<Value> derefId = GetVar(a_id);
		if (!derefId) {
			Logger::Log(Logger::LL::Error) << "Could not find variable " << a_id << std::endl;
			return 1;
		}
		return unary_op(a_id, derefId.value(), func);
	}

	i32 Routine::binary_op(u32 id, Value a, Value b, std::function<Value(Value, Value)> func) {
		return try_set(id, func(a, b));
	}

	i32 Routine::self_binary_op(u32 a_id, Value b, std::function<Value(Value, Value)> func) {
		std::optional<Value> derefId = GetVar(a_id);
		if (!derefId) {
			Logger::Log(Logger::LL::Error) << "Could not find variable " << a_id << std::endl;
			return 1;
		}
		return binary_op(a_id, derefId.value(), b, func);
	}

#ifdef _DEBUG
	void Routine::DebugDisassemble() const {
		Logger::Log(Logger::LL::Debug) << "offset   time  diff rank   name             args" << std::endl;
		Ins ins;
		for (u32 offset = 0; offset < code.size(); offset += static_cast<u32>(ins.size())) {
			ins = Ins(code.begin() + offset);
			ins.DebugDisassemble(offset);
		}
	}
#endif // _DEBUG


	//-----------------------------------
	//            RoutineBase
	//-----------------------------------

	void RoutineBase::InitializeDefVars() {
		std::vector<VarInitializeInfo> defvars = {
			//  {vtid, initial, read_only, inherit, pass}
				{VTID::I0, 0, false, true, VTID::NIL},
				{VTID::I1, 0, false, true, VTID::NIL},
				{VTID::I2, 0, false, true, VTID::NIL},
				{VTID::I3, 0, false, true, VTID::NIL},
				{VTID::I4, 0, false, true, VTID::NIL},
				{VTID::I5, 0, false, true, VTID::NIL},
				{VTID::I6, 0, false, true, VTID::NIL},
				{VTID::I7, 0, false, true, VTID::NIL},
				{VTID::LI0, 0, false, false, VTID::NIL},
				{VTID::LI1, 0, false, false, VTID::NIL},
				{VTID::LI2, 0, false, false, VTID::NIL},
				{VTID::LI3, 0, false, false, VTID::NIL},
				{VTID::LI4, 0, false, false, VTID::NIL},
				{VTID::LI5, 0, false, false, VTID::NIL},
				{VTID::LI6, 0, false, false, VTID::NIL},
				{VTID::LI7, 0, false, false, VTID::NIL},
				{VTID::F0, 0, false, true, VTID::NIL},
				{VTID::F1, 0, false, true, VTID::NIL},
				{VTID::F2, 0, false, true, VTID::NIL},
				{VTID::F3, 0, false, true, VTID::NIL},
				{VTID::F4, 0, false, true, VTID::NIL},
				{VTID::F5, 0, false, true, VTID::NIL},
				{VTID::F6, 0, false, true, VTID::NIL},
				{VTID::F7, 0, false, true, VTID::NIL},
				{VTID::LF0, 0, false, false, VTID::NIL},
				{VTID::LF1, 0, false, false, VTID::NIL},
				{VTID::LF2, 0, false, false, VTID::NIL},
				{VTID::LF3, 0, false, false, VTID::NIL},
				{VTID::LF4, 0, false, false, VTID::NIL},
				{VTID::LF5, 0, false, false, VTID::NIL},
				{VTID::LF6, 0, false, false, VTID::NIL},
				{VTID::LF7, 0, false, false, VTID::NIL},
				{VTID::IN0, 0, false, false, VTID::OUT0},
				{VTID::IN1, 0, false, false, VTID::OUT1},
				{VTID::IN2, 0, false, false, VTID::OUT2},
				{VTID::IN3, 0, false, false, VTID::OUT3},
				{VTID::IN4, 0, false, false, VTID::OUT4},
				{VTID::IN5, 0, false, false, VTID::OUT5},
				{VTID::IN6, 0, false, false, VTID::OUT6},
				{VTID::IN7, 0, false, false, VTID::OUT7},
				{VTID::OUT0, 0, false, false, VTID::NIL},
				{VTID::OUT1, 0, false, false, VTID::NIL},
				{VTID::OUT2, 0, false, false, VTID::NIL},
				{VTID::OUT3, 0, false, false, VTID::NIL},
				{VTID::OUT4, 0, false, false, VTID::NIL},
				{VTID::OUT5, 0, false, false, VTID::NIL},
				{VTID::OUT6, 0, false, false, VTID::NIL},
				{VTID::OUT7, 0, false, false, VTID::NIL},
				{VTID::RAND, 0, true, false, VTID::NIL},
				{VTID::RANDF, 0, true, false, VTID::NIL},
				{VTID::RANDF2, 0, true, false, VTID::NIL},
				{VTID::RANDRAD, 0, true, false, VTID::NIL},
				{VTID::TIME, -1, true, false, VTID::NIL},
				{VTID::CLOCK, -1, false, false, VTID::NIL},
		};
		InitializeVars(defvars);
	}

	std::optional<Value> RoutineBase::GetVar(u32 id) {
		switch (id) {
		case VTID::RAND: return GetVarRef(id).transform([](auto varRef) {return varRef.get() = ZDrive::rand(); });
		case VTID::RANDF: return GetVarRef(id).transform([](auto varRef) {return varRef.get() = ZDrive::randf(); });
		case VTID::RANDF2: return GetVarRef(id).transform([](auto varRef) {return varRef.get() = ZDrive::randf2(); });
		case VTID::RANDRAD: return GetVarRef(id).transform([](auto varRef) {return varRef.get() = ZDrive::randRad(); });
		}
		return Routine::GetVar(id);
	}

	std::unique_ptr<Routine> RoutineBase::Clone(u32 newInstanceId) const {
		std::unique_ptr<Routine> clone = std::make_unique<RoutineBase>(*this);
		(clone.get())->*(&RoutineBase::instanceId) = newInstanceId;
		return clone;
	}

	Routine::HandleResult RoutineBase::Handle(ProcessedInstruction const& ins) {
		HandleResult ret{true, false, false};
		u32 opcode = ins.opcode;
		std::vector<Value> const& args = ins.args;
		Value& clock = vt[VTID::CLOCK].val;

		try {
			switch (opcode) {
			case INS::NOP: break;
			case INS::RET: ret.shouldReturn = ret.deleteMe = true; break;
			case INS::WAIT: clock.s -= args.at(0).s; break;
			case INS::JMP: OP_jmp(args.at(0), args.at(1)); break;
			case INS::LOOP: {
				u32 iterId = args.at(2);
				auto iterOpt = GetVar(iterId);
				if (!iterOpt) {
					Logger::Log(Logger::LL::Error) << "Could not find variable " << iterId << std::endl;
					ret.success = false;
					break;
				}
				Value iter = iterOpt.value();
				if (iter.u) {
					OP_jmp(args.at(0), args.at(1));
					iter.u--;
					i32 result = SetVar(iterId, iter);
					if (result == 2) {
						Logger::Log(Logger::LL::Error) << "Variable " << iterId << " is read only." << std::endl;
						ret.success = false;
					}
				}
				break;
			}
			case INS::SET: ret.success = !try_set(args.at(0), args.at(1)); break;
			case INS::ISET: ret.success = !try_set(args.at(0), static_cast<i32>(args.at(1).f)); break;
			case INS::FSET: ret.success = !try_set(args.at(0), static_cast<f32>(args.at(1).s)); break;
			case INS::ISET_RAND_SIGN: ret.success = !try_set(args.at(0), randSignS(args.at(1))); break;
			case INS::FSET_RAND_SIGN: ret.success = !try_set(args.at(0), randSignF(args.at(1).f)); break;
			case INS::IADD: ret.success = !self_binary_op(args.at(0), args.at(1), func_iadd); break;
			case INS::ISUB: ret.success = !self_binary_op(args.at(0), args.at(1), func_isub); break;
			case INS::IMUL: ret.success = !self_binary_op(args.at(0), args.at(1), func_imul); break;
			case INS::IDIV: ret.success = !self_binary_op(args.at(0), args.at(1), func_idiv); break;
			case INS::IMOD: ret.success = !self_binary_op(args.at(0), args.at(1), func_imod); break;
			case INS::IMOD2: ret.success = !self_binary_op(args.at(0), args.at(1), func_imod2); break;
			case INS::FADD: ret.success = !self_binary_op(args.at(0), args.at(1), func_fadd); break;
			case INS::FSUB: ret.success = !self_binary_op(args.at(0), args.at(1), func_fsub); break;
			case INS::FMUL: ret.success = !self_binary_op(args.at(0), args.at(1), func_fmul); break;
			case INS::FDIV: ret.success = !self_binary_op(args.at(0), args.at(1), func_fdiv); break;
			case INS::FMOD: ret.success = !self_binary_op(args.at(0), args.at(1), func_fmod); break;
			case INS::FMOD2: ret.success = !self_binary_op(args.at(0), args.at(1), func_fmod2); break;
			case INS::ISET_ADD: ret.success = !binary_op(args.at(0), args.at(1), args.at(2), func_iadd); break;
			case INS::ISET_SUB: ret.success = !binary_op(args.at(0), args.at(1), args.at(2), func_isub); break;
			case INS::ISET_MUL: ret.success = !binary_op(args.at(0), args.at(1), args.at(2), func_imul); break;
			case INS::ISET_DIV: ret.success = !binary_op(args.at(0), args.at(1), args.at(2), func_idiv); break;
			case INS::ISET_MOD: ret.success = !binary_op(args.at(0), args.at(1), args.at(2), func_imod); break;
			case INS::FSET_ADD: ret.success = !binary_op(args.at(0), args.at(1), args.at(2), func_fadd); break;
			case INS::FSET_SUB: ret.success = !binary_op(args.at(0), args.at(1), args.at(2), func_fsub); break;
			case INS::FSET_MUL: ret.success = !binary_op(args.at(0), args.at(1), args.at(2), func_fmul); break;
			case INS::FSET_DIV: ret.success = !binary_op(args.at(0), args.at(1), args.at(2), func_fdiv); break;
			case INS::FSET_MOD: ret.success = !binary_op(args.at(0), args.at(1), args.at(2), func_fmod); break;
			case INS::IINC: ret.success = !self_binary_op(args.at(0), 1, func_iadd); break;
			case INS::FINC: ret.success = !self_binary_op(args.at(0), 1, func_fadd); break;
			case INS::IDEC: ret.success = !self_binary_op(args.at(0), 1, func_isub); break;
			case INS::FDEC: ret.success = !self_binary_op(args.at(0), 1, func_fsub); break;
			case INS::FSET_SIN: ret.success = !unary_op(args.at(0), args.at(1), func_sin); break;
			case INS::FSET_COS: ret.success = !unary_op(args.at(0), args.at(1), func_cos); break;
			case INS::FSET_TAN: ret.success = !unary_op(args.at(0), args.at(1), func_tan); break;
			case INS::FSET_ANGLE: {
				u32 id = args.at(0);
				f32 x1 = args.at(1);
				f32 y1 = args.at(2);
				f32 x2 = args.at(3);
				f32 y2 = args.at(4);
				ret.success = !try_set(id, atan2f(y1 - y2, x1 - x2));
				break;
			}
			case INS::FINTERP: {
				u32 id = args.at(0);
				u32 t = args.at(1);
				u32 m = args.at(2);
				f32 start = args.at(3);
				f32 end = args.at(4);
				f32 f1 = args.at(5);
				f32 f2 = args.at(6);

				auto interp_optref = vm.CloneAndActivateTemplate(DEF_SUB::INTERP_LIN + m);
				if (interp_optref) {
					Routine& interp_rt = interp_optref.value();
					// dumb hack because i think protected methods should be accessible through their objects within derived classes
					// ret.success &= !interp_rt.try_set(VTID::IN0, ValPtr(instanceId, id)); // Error: protected function is not accessible through member or pointer
					ret.success &= !(interp_rt.*(&RoutineBase::try_set))(VTID::IN0, ValPtr(instanceId, id));
					ret.success &= !(interp_rt.*(&RoutineBase::try_set))(VTID::IN1, t);
					ret.success &= !(interp_rt.*(&RoutineBase::try_set))(VTID::IN2, m);
					ret.success &= !(interp_rt.*(&RoutineBase::try_set))(VTID::IN3, start);
					ret.success &= !(interp_rt.*(&RoutineBase::try_set))(VTID::IN4, end);
					ret.success &= !(interp_rt.*(&RoutineBase::try_set))(VTID::IN5, f1);
					ret.success &= !(interp_rt.*(&RoutineBase::try_set))(VTID::IN6, f2);
				} else {
					Logger::Log(Logger::LL::Error) << "Could not find routine with subId " << DEF_SUB::INTERP_LIN + m << "in templates." << std::endl;
					ret.success = false;
				}
				break;
			}
			case INS::NORMRAD: ret.success = !self_unary_op(args.at(0), [](Value a) { return remainderf(a, static_cast<f32>(M_PI*2)) - static_cast<f32>(M_PI); }); break;
			case INS::MATHCIRCLEPOS: {
				u32 idX = args.at(0);
				u32 idY = args.at(1);
				f32 r = args.at(2);
				f32 theta = args.at(3);
				ret.success &= !try_set(idX, r*cosf(theta));
				ret.success &= !try_set(idY, r*sinf(theta));
				break;
			}
			case INS::MATHDISTANCE: {
				u32 id = args.at(0);
				f32 dx = args.at(3).f - args.at(1).f;
				f32 dy = args.at(4).f - args.at(2).f;
				ret.success = !try_set(id, sqrtf(dx*dx + dy*dy));
				break;
			}
			case INS::JMP_EQU: if (args.at(2) == args.at(3)) OP_jmp(args.at(0), args.at(1)); break;
			case INS::JMP_EQU_F: if (fabsf(args.at(2).f - args.at(3).f) < fabsf(args.at(4))) OP_jmp(args.at(0), args.at(1)); break;
			case INS::JMP_NEQ: if (args.at(2) != args.at(3)) OP_jmp(args.at(0), args.at(1)); break;
			case INS::JMP_NEQ_F: if (fabsf(args.at(2).f - args.at(3).f) > fabsf(args.at(4))) OP_jmp(args.at(0), args.at(1)); break;
			case INS::JMP_LT: if (args.at(2).s < args.at(3).s) OP_jmp(args.at(0), args.at(1)); break;
			case INS::JMP_LT_F: if (args.at(2).f < args.at(3).f) OP_jmp(args.at(0), args.at(1)); break;
			case INS::JMP_LTE: if (args.at(2).s <= args.at(3).s) OP_jmp(args.at(0), args.at(1)); break;
			case INS::JMP_LTE_F: if (args.at(2).f <= args.at(3).f + copysignf(args.at(4), args.at(3))) OP_jmp(args.at(0), args.at(1)); break;
			case INS::JMP_GT: if (args.at(2).s > args.at(3).s) OP_jmp(args.at(0), args.at(1)); break;
			case INS::JMP_GT_F: if (args.at(2).f > args.at(3).f) OP_jmp(args.at(0), args.at(1)); break;
			case INS::JMP_GTE: if (args.at(2).s >= args.at(3).s) OP_jmp(args.at(0), args.at(1)); break;
			case INS::JMP_GTE_F: if (args.at(2).f >= args.at(3).f - copysignf(args.at(4), args.at(3))) OP_jmp(args.at(0), args.at(1)); break;
			case INS::CALL: {
				auto rt_optref = vm.CloneAndActivateTemplate(args.at(0));
				if (!rt_optref) {
					Logger::Log(Logger::LL::Error) << "Could not find routine with subId " << args.at(0).u << "in templates." << std::endl;
					ret.success = false;
				}
				break;
			}
			case INS::YEILD: ret.shouldReturn = true; break;
			case INS::PRINT: {
				Logger::LogLevel level = static_cast<Logger::LogLevel>(args.at(0).s);
				Logger::Log(level) << toStringShortened() << " " << args.at(2).toString(args.at(1)) << std::endl;
				break;
			}
			case INS::SET_PTR: try_set(args.at(0), ValPtr(instanceId, args.at(1))); break;
			case INS::SET_PRIORITY: vm.UpdatePriority(instanceId, args.at(0)); break;
			}
		} catch (std::out_of_range oor) {
			Logger::Log(Logger::LL::Error) << "Fatal error on {instance: " << instanceId << ", sub: " << subId << ", type: " << GetTypeID() << "}:" << std::endl;
			Logger::Log(Logger::LL::Error) << "Not enough arguments for opcode " << opcode << ": only " << args.size() << " arguments found." << std::endl;
			ret.success = false;
			ret.shouldReturn = true;
			ret.deleteMe = true;
		}
		return ret;
	}

	void RoutineBase::OP_jmp(u32 pos, i32 t) {
		nextPtr = pos; 
		vt[VTID::CLOCK].val = t;
	}

}