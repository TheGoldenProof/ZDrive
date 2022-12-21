#pragma once

namespace ZDrive::VM {
	class ZVM : public IHasVarTable {
	public:
		const std::vector<i32> code;

		// will push the following error codes to results:
		// 1: unrecognized routine type
		// 2: routine count is zero
		// 3: no mainId was set
		// 4: no routine with id mainId was found
		ZVM(std::vector<i32>&& code, std::vector<i32>& results);
		~ZVM();

		inline bool IsFinished() const { return finished; }

		// returns true if there were no errors
		bool Update();

		// returns nullopt if instanceId == 0
		std::optional<std::reference_wrapper<Routine>> GetRoutineByInstance(u32 instanceId);
		// return asker if instanceId == 0
		std::optional<std::reference_wrapper<Routine>> GetRoutineByInstance(u32 instanceId, Routine& asker);

		// 0: success
		// 1: vtid not found
		// 2: vtid is read-only
		// 3: instance not found
		i32 SetVarByPtr(ValPtr id, Value value, Routine& asker);
		std::optional<Value> GetVarByPtr(ValPtr id, Routine& asker);
		std::optional<std::reference_wrapper<Value>> GetVarRefByPtr(ValPtr id, Routine& asker);

		virtual void InitializeDefVars() override;

		std::optional<std::reference_wrapper<Routine>> CloneAndActivateTemplate(u32 subId);
		void UpdatePriority(u32 instance, i32 newPriority);

#ifdef _DEBUG
		void DebugDisassemble() const;
#endif // _DEBUG

	private:
		ZVM();

		bool finished = false;
		u32 instanceTracker = 1;

		std::vector<std::unique_ptr<Routine>> templates;
		std::set<std::unique_ptr<Routine>, Routine::Compare> active;
		std::vector<Routine*> toBeResorted;

		void ResortActive();
	};
}