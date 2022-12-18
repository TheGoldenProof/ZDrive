#include "ZDriveCompiler.hpp"

#include "compiler.hpp"

namespace ZDrive::Compiler {
	std::vector<i32> Compile(LanguageDeclaration const& langDecl, std::string const& source, std::string const& entrySubName) {
		return _Compiler(langDecl, source, entrySubName)();
	}
}