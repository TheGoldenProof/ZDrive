#pragma once

namespace ZDrive {
	namespace Logger {
		using LTF = enum class TimeFormat {
			// Use seconds since January 1st, 1970 UTC
			UTC_RAW,
			// Use formatted time since January 1st, 1970 UTC
			UTC_F,
			// Use seconds since January 1st, 1970 in the local time zone
			LCL_RAW,
			// Use formatted time since January 1st, 1970 in the local time zone
			LCL_F,

			// specifies which mode should be default
			DEFAULT = LCL_F,
		};

		using LL = enum class LogLevel {
			All = -1,
			Debug,
			Info,
			Warn,
			Error,
			Fatal,
			None,
		};

		constexpr const char* LogLevelToString(LogLevel _level);

		/// <summary>
		/// Initalizes the logger, including but not exclusively:
		/// Sets the level and output destination and starts the clock.
		/// Not necessary, but recommended to call before Log(). 
		/// </summary>
		/// <param name="level">Messages below this level will not be output.</param>
		/// <param name="outputDest">Where the log should output to.</param>
		void Initialize(LogLevel level, std::ostream& outputDest);
		/// <summary></summary>
		/// <param name="level">Messages below this level will not be output.</param>
		void SetLevel(LogLevel level);
		/// <summary></summary>
		/// <param name="outputDest">Where the log should output to.</param>
		void SetOutput(std::ostream& outputDest);
		/// <summary>
		/// Resets the high-resolution time source.
		/// </summary>
		void ResetClock();
		/// <summary>
		/// Prepends some information to the stream and then return it if the passed level is greater or equal to the set level.
		/// If the logger has not been initialized, Initialize() will be called first with the current level (default: Error for Release, All for Debug) and output stream (default: std::cout).
		/// </summary>
		/// <param name="level">The level to output subsequent messages as.</param>
		/// <returns>The output stream initialized with if 'level' is greater or equal to the level the logger is set to. Otherwise, TGLib::nullout.</returns>
		std::ostream& Log(LogLevel level);
	}
}