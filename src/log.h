#pragma once

#ifndef PTR_DIST
#	define LOG_TRACE(...) ::log::get_log()->trace(__VA_ARGS__)
#	define LOG_INFO(...)  ::log::get_log()->info(__VA_ARGS__)
#	define LOG_WARN(...)  ::log::get_log()->warn(__VA_ARGS__)
#	define LOG_ERROR(...) ::log::get_log()->error(__VA_ARGS__)
#	define ASSERT(x, ...)          \
		if (!(x))                   \
		{                           \
			LOG_ERROR(__VA_ARGS__); \
			__debugbreak();         \
		}
#else
#	define LOG_TRACE(...)
#	define LOG_INFO(...)
#	define LOG_WARN(...)
#	define LOG_ERROR(...)
#	define ASSERT(x, ...)
#endif
#define LOG_LEVEL(L) ::log::set_level(spdlog::level::level_enum::L)

class log final
{
public:
	static spdlog::logger* get_log()
	{
		if (!s_log)
			init();
		return s_log;
	}
	static void set_level(const spdlog::level::level_enum level)
	{
		if (!s_log)
			init();
		s_log->set_level(level);
	}
private:
	static inline spdlog::logger* s_log = nullptr;
private:
	static void init()
	{
		spdlog::set_pattern("%^[%T | %n]: %v%$");
		s_log = spdlog::stdout_color_mt("PHORM").get();
		s_log->set_level(spdlog::level::trace);
	}
};
