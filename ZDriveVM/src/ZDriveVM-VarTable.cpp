#include "ZDriveVM.hpp"

namespace ZDrive::VM {

	void IHasVarTable::InitializeVars(std::vector<VarInitializeInfo> const& vars) {
		vt.reserve(vt.size() + vars.size());
		std::for_each(vars.begin(), vars.end(), [this](VarInitializeInfo const& info) { vt[info.id] = info.var; });
	}

	void IHasVarTable::ClearVars() {
		vt.clear();
	}

	i32 IHasVarTable::SetVar(u32 id, Value val) {
		auto res = vt.find(id);
		if (res == vt.end()) return 1;
		if (res->second.read_only) return 2;
		res->second.val = val;
		return 0;
	}

	std::optional<Value> IHasVarTable::GetVar(u32 id) {
		//return GetVarRef(id).transform([](std::reference_wrapper<Value> const& ref) {return ref.get(); });
		auto res = vt.find(id);
		if (res == vt.end()) return std::nullopt;
		return res->second.val;
	}

	std::optional<std::reference_wrapper<Value>> IHasVarTable::GetVarRef(u32 id) {
		auto res = vt.find(id);
		if (res == vt.end()) return std::nullopt;
		return res->second.val;
	}

	// if (this->var.inhert) this->var.value = other.var.value
	void IHasVarTable::InheritVarsFrom(IHasVarTable& other) {
		for (auto& [id, var] : vt) {
			// i had to do this weird if sequence because i didn't know if it would call GetVarRef before short-circuiting if i combined them. I don't want it to do that.
			if (var.inherit) if (auto varoptref = GetVarRef(id); varoptref) if (const auto other_varopt = other.GetVar(id); other_varopt) {
				varoptref.value().get() = Value(other_varopt.value());
			}
		}
	}
	// if (this->var.inhert) other.var.value = this->var.value
	void IHasVarTable::InheritVarsTo(IHasVarTable& other) { 
		for (auto& [id, var] : vt) {
			if (var.inherit) if (const auto varopt = GetVar(id); varopt) if (auto other_varoptref = other.GetVarRef(id); other_varoptref) {
				other_varoptref.value().get() = Value(varopt.value());
			}
		}
	}
	// if (this->var.pass) this->var.value = other.var.value
	void IHasVarTable::PassVarsFrom(IHasVarTable& other) { 
		for (auto& [id, var] : vt) {
			if (var.pass_dest) if (auto varoptref = GetVarRef(id); varoptref) if (const auto other_varopt = other.GetVar(id); other_varopt) {
				varoptref.value().get() = Value(other_varopt.value());
			}
		}
	}
	// if (this->var.pass) other.var.value = this->var.value
	void IHasVarTable::PassVarsTo(IHasVarTable& other) {
		for (auto& [id, var] : vt) {
			if (var.pass_dest) if (const auto varopt = GetVar(id); varopt) if (auto other_varoptref = other.GetVarRef(id); other_varoptref) {
				other_varoptref.value().get() = Value(varopt.value());
			}
		}
	}
}