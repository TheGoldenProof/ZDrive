#pragma once

namespace ZDrive::VM {
	struct VarTableVar {
		Value val;
		bool read_only;
		bool inherit;
		u32 pass_dest; // use VTID::NIL to not pass

		VarTableVar(Value val = Value(), bool read_only = false, bool inherit = false, u32 pass_dest = VTID::NIL) : val(val), read_only(read_only), inherit(inherit), pass_dest(pass_dest) {}
	};

	struct VarInitializeInfo {
		u32 id;
		VarTableVar var;

		VarInitializeInfo(u32 id, Value initial = Value(), bool read_only = false, bool inherit = false, u32 pass_dest = VTID::NIL) : id(id), var(initial, read_only, inherit, pass_dest) {}
		VarInitializeInfo(u32 id, VarTableVar var) : id(id), var(var) {}
	};

	class IHasVarTable {
	public:
		// Initializes the VarTable variables. Only initialized variables exist and this is the only way to make a variable exist within the VarTable.
		void InitializeVars(std::vector<VarInitializeInfo> const& vars);
		// Calls InitializeVars with a default set of variables.
		virtual void InitializeDefVars() {}
		// Removes all variables, making them no longer exist within the table.
		void ClearVars();
		// Sets a variable if it is generically writable (ie. can be set with 'set' instructions)(meaning it was initialized with read_only = false). Use GetVarRef to modify a generically read-only variable.
		// returns:
		// 0: success
		// 1: id not found
		// 2: id is read-only
		virtual i32 SetVar(u32 id, Value val);
		// Gets a variable by value
		virtual std::optional<Value> GetVar(u32 id);
		// Gets a variable by reference. Returns a modifyable reference as long as the variable exists, regardless of read_only status.
		virtual std::optional<std::reference_wrapper<Value>> GetVarRef(u32 id);

		// copies values marked as inherit on this from other to this.
		virtual void InheritVarsFrom(IHasVarTable& other);
		// copies values marked as inherit on this from this to other. Does not modify this.
		virtual void InheritVarsTo(IHasVarTable& other);
		// copies values marked as pass on this to this from other.
		virtual void PassVarsFrom(IHasVarTable& other);
		// copies values marked as pass on this to other from this. Does not modify this.
		virtual void PassVarsTo(IHasVarTable& other);
	protected:
		IHasVarTable() {}

		std::unordered_map<u32, VarTableVar> vt;
	};
}