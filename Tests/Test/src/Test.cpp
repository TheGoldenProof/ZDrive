#define WIN32_LEAN_AND_MEAN

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <Windows.h>

#include "ZDriveCompiler.hpp"
#include "ZDriveVM.hpp"

using TGLib::i32;
using TGLib::i64;
using TGLib::u32;
using TGLib::usize;

static void compileToFile(std::vector<std::string> inPaths, std::string outPath, std::string entryName);
static void runFile(std::string path);

int main(int argc, const char* argv[]) {
	std::vector<std::string> args(argv, argv + argc);

	ZDrive::Logger::Initialize(ZDrive::Logger::LL::All, std::cout);

	if (argc >= 5 && !args[1].compare("compile")) {
		compileToFile(std::vector<std::string>(&argv[3], &argv[argc - 1]), args[argc - 1], args[2]);
	} else if (argc == 3 && !args[1].compare("run")) {
		runFile(args[2]);
	} else {
		std::cerr << "Usage: run <inputPath>" << std::endl;
		std::cerr << "Usage: compile <entry func name> <input path 1> [...] [input path n] <output path>" << std::endl;
		exit(64);
	}

	return 0;
}

static std::string readFileToString(std::string spath) {
	namespace fs = std::filesystem;
	fs::path path(spath);
	std::ifstream f(path, std::ios::in | std::ios::binary);
	usize size = fs::file_size(path);
	std::string res(size, '\0');
	f.read(res.data(), size);
	return res;
}

static std::vector<i32> readFileToInts(std::string spath) {
	namespace fs = std::filesystem;
	fs::path path(spath);
	std::ifstream f(path, std::ios::in | std::ios::binary);
	usize size = fs::file_size(path);
	std::vector<i32> buf;
	buf.resize(size / sizeof(i32));
	f.read((char*)buf.data(), size);
	return buf;
}

static void compileToFile(std::vector<std::string> inPaths, std::string outPath, std::string entryName) {
	std::string source;
	for (std::string file : inPaths) {
		source.append(readFileToString(file));
	}

	ZDrive::Compiler::LangDecl lang;
	lang.DeclareDefaultBaseIns();

	std::vector<i32> result = ZDrive::Compiler::Compile(lang, source, entryName);

	if (result.size() == 0) { 
		ZDrive::Logger::Log(ZDrive::Logger::LL::Fatal) << "result.size() = 0. Compilation failed.";
		exit(65); return; 
	}

	std::fstream outputFile = std::fstream(outPath, std::ios::out | std::ios::binary);
	outputFile.write((const char*)(result.data()), result.size() * sizeof(i32));
	outputFile.close();
}

static void runFile(std::string path) {
	ZDrive::setRandSeed(12182022);
	ZDrive::Logger::Initialize(ZDrive::Logger::LL::All, std::cout);

	std::vector<i32> errors;
	ZDrive::VM::ZVM vm = ZDrive::VM::ZVM(readFileToInts(path), errors);

	if (auto diff = vm.GetVarRef(ZDrive::VTID::DIFF); diff) diff.value().get().s = 1;
	if (auto rank = vm.GetVarRef(ZDrive::VTID::RANK); rank) rank.value().get().s = 1;

#if 0
	vm.DebugDisassemble();
#endif

#if 1
	LARGE_INTEGER pcf;
	QueryPerformanceFrequency(&pcf);
	i64 target = pcf.QuadPart / 60;
	LARGE_INTEGER pcLastUpdate = { 0 };
	LARGE_INTEGER udpc = { 0 };
	LARGE_INTEGER pc = { 0 };
	QueryPerformanceCounter(&pc);
	udpc.QuadPart = pc.QuadPart;

	while (!vm.IsFinished()) {
		QueryPerformanceCounter(&pc);
		udpc.QuadPart = pc.QuadPart - pcLastUpdate.QuadPart;
		if (udpc.QuadPart >= target) {
			vm.Update();
			pcLastUpdate.QuadPart = pc.QuadPart;
		}
	}
#endif
}