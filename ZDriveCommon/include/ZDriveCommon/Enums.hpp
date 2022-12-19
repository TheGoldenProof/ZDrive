#pragma once

namespace ZDrive {
	//This namespace thing was a really late workaround to allow implicit integral conversions while keeping the EnumName::value format.

	namespace RoutineType {
		enum RT {
			BASE, ENTITY,
		};
	}
	namespace RT = RoutineType;

	namespace ValueType {
		enum VT {
			SINT, UINT, FLOAT, HEX, PTR
		};
	}
	namespace VT = ValueType;

	namespace ArgType {
		enum AT : i32 {
			CNST, VTREF,
			TEMP_LABEL = -1,
		};
	}
	namespace AT = ArgType;

	namespace InterpretResult {
		enum Interpret {
			OK, COMPILE_ERROR, RUNTIME_ERROR
		};
	}
	namespace Interpret = InterpretResult;

	namespace VarTableID {
		enum VTID : u32 {
			NIL, I0, I1, I2, I3, I4, I5, I6, I7,
			LI0, LI1, LI2, LI3, LI4, LI5, LI6, LI7,
			F0, F1, F2, F3, F4, F5, F6, F7,
			LF0, LF1, LF2, LF3, LF4, LF5, LF6, LF7,
			IN0, IN1, IN2, IN3, IN4, IN5, IN6, IN7,
			OUT0, OUT1, OUT2, OUT3, OUT4, OUT5, OUT6, OUT7,
			RAND, RANDF, RANDF2, RANDRAD,
			DIFF, RANK,
			TIME, CLOCK, LAST_BASE,

			ENT_SHOT = 256,
		};
	}
	namespace VTID = VarTableID;

	namespace BaseOpCode {
		enum INS : u32 {
			NOP = 0,
			RET,
			WAIT,
			JMP,
			LOOP,
			SET,
			ISET,
			FSET,
			ISET_RAND_SIGN,
			FSET_RAND_SIGN,
			IADD,
			ISUB,
			IMUL,
			IDIV,
			IMOD,
			IMOD2,
			FADD,
			FSUB,
			FMUL,
			FDIV,
			FMOD,
			FMOD2,
			ISET_ADD,
			ISET_SUB,
			ISET_MUL,
			ISET_DIV,
			ISET_MOD,
			FSET_ADD,
			FSET_SUB,
			FSET_MUL,
			FSET_DIV,
			FSET_MOD,
			IINC,
			FINC,
			IDEC,
			FDEC,
			FSET_SIN,
			FSET_COS,
			FSET_TAN,
			FSET_ANGLE,
			FINTERP,
			NORMRAD,
			MATHCIRCLEPOS,
			MATHDISTANCE,
			JMP_EQU,
			JMP_EQU_F,
			JMP_NEQ,
			JMP_NEQ_F,
			JMP_LT,
			JMP_LT_F,
			JMP_LTE,
			JMP_LTE_F,
			JMP_GT,
			JMP_GT_F,
			JMP_GTE,
			JMP_GTE_F,
			CALL,
			YEILD,
			PRINT,
			SET_PTR,
			ASSERT_PTR,
			SET_PRIORITY,

			BASE_FIRST = NOP,
			BASE_LAST = SET_PRIORITY,
		};
	}
	namespace INS = BaseOpCode;

	namespace DefaultSubID {
		enum DEF_SUB {
			INTERP_LINEAR = 0,
			INTERP_QUAD_IN, INTERP_CUBE_IN, INTERP_QUART_IN,
			INTERP_QUAD_OUT, INTERP_CUBE_OUT, INTERP_QUART_OUT,
			INTERP_CONST_VEL, INTERP_BEZIER,
			INTERP_QUAD_IO, INTERP_CUBE_IO, INTERP_QUART_IO,
			INTERP_QUAD_OI, INTERP_CUBE_OI, INTERP_QUART_OI,
			INTERP_DELAY, INTERP_INSTANT,
			INTERP_CONST_ACC,
			INTERP_SINE_OUT, INTERP_SINE_IN, INTERP_SINE_IO, INTERP_SINE_OI,
			INTERP_OVER_IN_A, INTERP_OVER_IN_B, INTERP_OVER_IN_C, INTERP_OVER_IN_D, INTERP_OVER_IN_E,
			INTERP_OVER_OUT_A, INTERP_OVER_OUT_B, INTERP_OVER_OUT_C, INTERP_OVER_OUT_D, INTERP_OVER_OUT_E,
		};
	}
	namespace DEF_SUB = DefaultSubID;

	namespace InterpMode {
		enum INTERP {
			LINEAR = 0,
			QUAD_IN, CUBE_IN, QUART_IN,
			QUAD_OUT, CUBE_OUT, QUART_OUT,
			CONST_VEL, BEZIER,
			QUAD_IO, CUBE_IO, QUART_IO,
			QUAD_OI, CUBE_OI, QUART_OI,
			DELAY, INSTANT,
			CONST_ACC,
			SINE_OUT, SINE_IN, SINE_IO, SINE_OI,
			OVER_IN_A, OVER_IN_B, OVER_IN_C, OVER_IN_D, OVER_IN_E,
			OVER_OUT_A, OVER_OUT_B, OVER_OUT_C, OVER_OUT_D, OVER_OUT_E,
		};
	}
	namespace INTERP = InterpMode;
}