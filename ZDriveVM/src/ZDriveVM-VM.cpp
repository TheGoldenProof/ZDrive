#include "ZDriveVM.hpp"

namespace ZDrive::VM {

	ZVM::ZVM(std::vector<i32>&& _code, std::vector<i32>& results) : code(std::move(_code)) {
		InitializeDefVars();

		u32 rt_count = code[0];
		u32 mainId = code[1];

		for (u32 i = 0; i < rt_count; i++) {
			u32 typeId = code[i * 3 + 2];
			u32 size = code[i * 3 + 3];
			u32 start = code[i * 3 + 4];

			Routine* rt_ptr = nullptr;
			
			switch (typeId) {
			default:
				Logger::Log(Logger::LL::Error) << "Error creating routine template for routine id " << i << ": type " << typeId << " not recognized.";
				results.push_back(1);
				[[fallthrough]];
			case RT::BASE: rt_ptr = new RoutineBase(*this, std::span(code.begin() + start, size), i, 0); break;
			}

			rt_ptr->Update();
			if (rt_ptr->deleteMe) {
				Logger::Log(Logger::LL::Error) << "Template for routine id " << i << " marked for deletion after construction. It will not be constructable.";
				delete rt_ptr;
			} else {
				templates.emplace_back(rt_ptr);
			}
		}

		if (rt_count == 0) {
			Logger::Log(Logger::LL::Warn) << "ZVM routine count is 0";
			finished = true;
			results.push_back(2);
		} else {
			if (mainId == static_cast<u32>(-1)) { // can i just say 0u-1?
				Logger::Log(Logger::LL::Warn) << "Code does not have an entry routine set and will not run.";
				finished = true;
				results.push_back(3);
			} else {
				auto res = CloneAndActivateTemplate(mainId);
				if (!res) results.push_back(4);
			}
		}
	}

	ZVM::~ZVM() {}

	bool ZVM::Update() {
		bool ret = true;

		if (active.size() == 0) {
			if (finished == true)
				Logger::Log(Logger::LL::Warn) << "ZVM Updated, but there are no active routines in the VM.";
			finished = true;
		}

		if (finished) {
			Logger::Log(Logger::LL::Info) << "The ZVM has finished running.";
		}

		for (auto&& iter = active.begin(); iter != active.end(); ) {
			std::unique_ptr<Routine> const& rt_ptr = *iter;
			if (rt_ptr->deleteMe) {
				iter = active.erase(iter);
			} else {
				if (!rt_ptr->Update()) {
					Logger::Log(Logger::LL::Error) << "Error updating routine " << rt_ptr->toString();
					ret = false;
				}
				iter++;
			}
		}

		if (toBeResorted.size()) {
			ResortActive();
		}

		vt[VTID::TIME].val.u++;
		return ret;
	}

	std::optional<std::reference_wrapper<Routine>> ZVM::GetRoutineByInstance(u32 instanceId) {
		if (instanceId == 0) return std::nullopt;
		for (std::unique_ptr<Routine> const& rt : active) {
			if (rt->GetInstanceID() == instanceId) return *rt;
		}
		return std::nullopt;
	}

	std::optional<std::reference_wrapper<Routine>> ZVM::GetRoutineByInstance(u32 instanceId, Routine& asker) {
		if (instanceId == 0) return asker;
		return GetRoutineByInstance(instanceId);
	}

	i32 ZVM::SetVarByPtr(ValPtr id, Value value, Routine& asker) {
		std::optional<std::reference_wrapper<Routine>> rt_optref = GetRoutineByInstance(id.b, asker);

		if (!rt_optref) {
			Logger::Log(Logger::LL::Warn) << "Routine with instanceId " << id.b << " not found.";
			return 3;
		}
		return rt_optref.value().get().SetVar(id.v, value);
	}

	std::optional<Value> ZVM::GetVarByPtr(ValPtr id, Routine& asker) {
		std::optional<std::reference_wrapper<Routine>> rt_optref = GetRoutineByInstance(id.b, asker);

		if (!rt_optref) {
			Logger::Log(Logger::LL::Warn) << "Routine with instanceId " << id.b << " not found.";
			return std::nullopt;
		}
		return rt_optref.value().get().GetVar(id.v);
	}

	std::optional<std::reference_wrapper<Value>> ZVM::GetVarRefByPtr(ValPtr id, Routine& asker) {
		std::optional<std::reference_wrapper<Routine>> rt_optref = GetRoutineByInstance(id.b, asker);

		if (!rt_optref) {
			Logger::Log(Logger::LL::Warn) << "Routine with instanceId " << id.b << " not found.";
			return std::nullopt;
		}
		return rt_optref.value().get().GetVarRef(id.v);
	}

	void ZVM::InitializeDefVars() {
		std::vector<VarInitializeInfo> defvars = {
		//  {vtid, initial, read_only, inherit, pass}
			{VTID::DIFF, 1, true, false, false},
			{VTID::RANK, 0, true, false, false},
			{VTID::TIME, -1, true, false, false},
			{VTID::ENT_SHOT, 0, true, false, false},
		};
		InitializeVars(defvars);
	}

	std::optional<std::reference_wrapper<Routine>> ZVM::CloneAndActivateTemplate(u32 subId) {
		try {
			std::unique_ptr<Routine> clone = templates.at(subId)->Clone(instanceTracker++);
			Routine& ret = *clone;
			active.insert(std::move(clone));
			return ret;
		} catch (std::out_of_range e) {
			Logger::Log(Logger::LL::Error) << "Could not clone " << subId << ": not found.";
			return std::nullopt;
		}
	}

	void ZVM::UpdatePriority(u32 instanceId, i32 newPriority) {
		if (instanceId == 0) return;
		for (std::unique_ptr<Routine> const& rt_uptr : active) {
			if (rt_uptr->GetInstanceID() == instanceId) {
				rt_uptr->SetPriority(newPriority);
				toBeResorted.push_back(rt_uptr.get());
				return;
			}
		}
		Logger::Log(Logger::LL::Error) << "Tried to update priority for non-existant instance: " << instanceId;
	}

#ifdef _DEBUG
	void ZVM::DebugDisassemble() const {
		u32 rt_count = code[0];
		u32 mainId = code[1];

		Logger::Log(Logger::LL::Debug) << "Routine Count: " << rt_count;
		Logger::Log(Logger::LL::Debug) << "Main Routine: " << mainId;

		for (u32 i = 0; i < templates.size(); i++) {
			u32 typeId = code[i * 3 + 2];
			u32 size = code[i * 3 + 3];
			u32 start = code[i * 3 + 4];
			Logger::Log(Logger::LL::Debug) << "------ Routine " << i << " ------";
			Logger::Log(Logger::LL::Debug) << "type: " << typeId << ", offset: " << start << ", size: " << size;
			templates[i]->DebugDisassemble();
		}
	}
#endif // _DEBUG

	void ZVM::ResortActive() {
		for (auto const& ptr : toBeResorted) {
			for (auto&& rt_uptr_iter = active.begin(); rt_uptr_iter != active.end(); rt_uptr_iter++) {
				if ((*rt_uptr_iter)->GetInstanceID() == ptr->GetInstanceID()) {
					std::unique_ptr<Routine> rt_uptr = std::move(active.extract(rt_uptr_iter).value());
					active.insert(std::move(rt_uptr));
					break;
				}
			}
		}
		toBeResorted.clear();
	}

}