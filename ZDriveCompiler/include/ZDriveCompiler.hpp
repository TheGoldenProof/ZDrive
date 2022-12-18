#pragma once

#include "ZDriveCommon.hpp"

#include <cctype>

#include "ZDriveCompiler/Lang.hpp"

namespace ZDrive::Compiler {
	/// <summary></summary>
	/// <param name="langDecl">The instruction mappings to use.</param>
	/// <param name="source">The code to compile.</param>
	/// <param name="entrySubName">The name of the inital sub to be called when the program is run. Defaults to "main".</param>
	/// <returns>The binary compiled code.</returns>
	std::vector<i32> Compile(LanguageDeclaration const& langDecl, std::string const& source, std::string const& entrySubName = "main");
}