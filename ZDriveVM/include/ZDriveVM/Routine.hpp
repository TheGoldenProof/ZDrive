#pragma once

namespace ZDrive::VM {
	class ZVM;

	struct ProcessedInstruction {
		u32 opcode = 0;
		std::vector<Value> args;
	};
	using PrxIns = ProcessedInstruction;

	class Routine : public IHasVarTable {
	public:
		class Compare {
		public:
			bool operator()(Routine const& r1, Routine const& r2) const { return r1.priority > r2.priority || (r1.priority == r2.priority && r1.instanceId < r2.instanceId); }
			bool operator()(std::unique_ptr<Routine> const& r1, std::unique_ptr<Routine> const& r2) const { return operator()(*r1, *r2); }
		};

		struct HandleResult {
			bool success;
			bool shouldReturn;
			bool deleteMe;
		};

		bool deleteMe = false;

		Routine(ZVM& vm, std::span<const i32> code, u32 subId, u32 instanceId) : vm(vm), code(code), subId(subId), instanceId(instanceId) { InitializeDefVars(); }
		virtual ~Routine() {}

		virtual std::string toString();
		virtual std::string toStringShortened();

		virtual std::unique_ptr<Routine> Clone(u32 newInstanceId) const = 0;
		
		virtual i32 SetVar(u32 id, Value val) override;
		virtual std::optional<Value> GetVar(u32 id) override;
		virtual std::optional<std::reference_wrapper<Value>> GetVarRef(u32 id) override;

		virtual HandleResult Handle(ProcessedInstruction const&) = 0;
		
		inline virtual u32 GetTypeID() const = 0;
		inline u32 GetSubID() const { return subId; }
		inline u32 GetInstanceID() const { return instanceId; }
		inline i32 GetPriority() const { return priority; }
		// Do not call this. This is for internal use only by ZVM::UpdatePriority. Use that to change the priority instead.
		inline void SetPriority(i32 newPriority) { newPriority = priority; }

		std::optional<ProcessedInstruction> ProcessInstruction(Instruction const& ins);
		std::optional<Value> ResolveArg(Arg const& arg);

		bool Update();

#ifdef _DEBUG
		void DebugDisassemble() const;
#endif // _DEBUG


	protected:
		ZVM& vm;
		const std::span<const i32> code;

		u32 subId;
		u32 instanceId;
		u32 ptr = 0;
		u32 nextPtr = 0;

		i32 try_set(u32 id, Value const& val);
		i32 unary_op(u32 id, Value a, std::function<Value(Value)> func);
		i32 self_unary_op(u32 a_id, std::function<Value(Value)> func);
		i32 binary_op(u32 id, Value a, Value b, std::function<Value(Value, Value)> func);
		i32 self_binary_op(u32 a_id, Value b, std::function<Value(Value, Value)> func);
	private:
		i32 priority = 0;
	};

	class RoutineBase : public Routine {
	public:
		RoutineBase(ZVM& vm, std::span<const i32> code, u32 subId, u32 instanceId) : Routine(vm, code, subId, instanceId) { InitializeDefVars(); }

		virtual void InitializeDefVars() override;
		virtual std::optional<Value> GetVar(u32 id) override;

		inline constexpr u32 GetTypeIDStatic() const { return 0; }
		inline virtual u32 GetTypeID() const override { return GetTypeIDStatic(); }
		virtual std::unique_ptr<Routine> Clone(u32 newInstanceId) const override;

		virtual HandleResult Handle(ProcessedInstruction const&) override;
	protected:
		static const std::function<Value(Value, Value)> func_iadd;
		static const std::function<Value(Value, Value)> func_isub;
		static const std::function<Value(Value, Value)> func_imul;
		static const std::function<Value(Value, Value)> func_idiv;
		static const std::function<Value(Value, Value)> func_imod;
		static const std::function<Value(Value, Value)> func_imod2;
		static const std::function<Value(Value, Value)> func_fadd;
		static const std::function<Value(Value, Value)> func_fsub;
		static const std::function<Value(Value, Value)> func_fmul;
		static const std::function<Value(Value, Value)> func_fdiv;
		static const std::function<Value(Value, Value)> func_fmod;
		static const std::function<Value(Value, Value)> func_fmod2;
		static const std::function<Value(Value)> func_sin;
		static const std::function<Value(Value)> func_cos;
		static const std::function<Value(Value)> func_tan;

		void OP_jmp(u32 pos, i32 t);
	};
}