#include "ZDriveCommon.hpp"

namespace ZDrive {
	using namespace Logger;
	
	class _Logger {
	public:
		static constexpr char const* ltfFormatString = "%x %X";
		static constexpr usize prefixStrBufferSize = 64;

		_Logger() : os(std::cout.rdbuf()) {}
		_Logger(std::ostream& os) : os(os.rdbuf()) {}

		bool initialized = false;
		char prefixStr[prefixStrBufferSize];
		std::ostream os;
		LARGE_INTEGER freq = { 0 };
		LARGE_INTEGER start_t = { 0 };
#ifdef _DEBUG
		LogLevel level = LL::All;
#else
		LogLevel level = LL::Error;
#endif // _DEBUG

		//inline static _Logger& instance { static _Logger _instance; return _instance; }
		void _Initialize(LogLevel _level, std::ostream& outputDest) {
			SetLevel(_level);
			SetOutput(outputDest);

			QueryPerformanceFrequency(&freq);

			ResetClock();

			initialized = true;
		}

		void _SetLevel(LogLevel _level) {
			level = _level;
		}

		void _SetOutput(std::ostream& outputDest) {
			os.rdbuf(outputDest.rdbuf());
		}

		void _ResetClock() {
			QueryPerformanceCounter(&start_t);
		}

		std::ostream& _Log(LogLevel _level) {
			if (!initialized) _Initialize(level, os);
			if (_level < level) return TGLib::nullout;
			//os << std::endl;
			return os << "[" << getPrefixString(LTF::DEFAULT, _level) << "] ";
		}

	private:

		const char* getPrefixString(TimeFormat ltf, LogLevel _level) {
			time_t tt = { 0 };
			tm tms;
			LARGE_INTEGER cur;
			QueryPerformanceCounter(&cur);
			LARGE_INTEGER ct0; ct0.QuadPart = cur.QuadPart - start_t.QuadPart;
			LARGE_INTEGER ct; ct.QuadPart = ct0.QuadPart * 1000000 / freq.QuadPart;
			char cts[32];
			memset(prefixStr, 0, prefixStrBufferSize * sizeof(char));

			sprintf_s(cts, _countof(cts), " | %010lld | %s", ct.QuadPart, Logger::LogLevelToString(_level));

			switch (ltf) {
			case LTF::UTC_RAW:
				tt = time(NULL);
				sprintf_s(prefixStr, _countof(prefixStr), "%lld", tt);
				break;
			case LTF::UTC_F:
				tt = time(NULL);
				gmtime_s(&tms, &tt);
				strftime(prefixStr, _countof(prefixStr), ltfFormatString, &tms);
				break;
			case LTF::LCL_RAW:
				tt = time(NULL);
				localtime_s(&tms, &tt);
				sprintf_s(prefixStr, _countof(prefixStr), "%lld", mktime(&tms));
				break;
			default:
			case LTF::LCL_F:
				tt = time(NULL);
				localtime_s(&tms, &tt);
				strftime(prefixStr, _countof(prefixStr), ltfFormatString, &tms);
				break;
			}

			strcat_s(prefixStr, _countof(prefixStr), cts);
			return prefixStr;
		}
	} instance;

	constexpr const char* Logger::LogLevelToString(LogLevel _level) {
		switch (_level) {
		case LL::Debug: return "DEBUG";
		case LL::Info: return "INFO";
		case LL::Warn: return "WARN";
		case LL::Error: return "ERROR";
		case LL::Fatal: return "FATAL";
		case LL::None:
		case LL::All:
		default: return "LOG";
		}
	}

	void Logger::Initialize(LogLevel level, std::ostream& outputDest) { return instance._Initialize(level, outputDest); }
	void Logger::SetLevel(LogLevel level) { return instance._SetLevel(level); }
	void Logger::SetOutput(std::ostream& outputDest) { return instance._SetOutput(outputDest); }
	void Logger::ResetClock() { return instance._ResetClock(); }
	std::ostream& Logger::Log(LogLevel level) { return instance._Log(level); }
}