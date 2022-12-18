#pragma once

namespace ZDrive {
	template <typename InsInfo>
	class LanguageBase {
	protected:
		std::unordered_set<InsInfo> instructions;

	public:
		LanguageBase() {}

		virtual ~LanguageBase() {}
	};
}