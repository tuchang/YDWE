#pragma once

#include <string>
#include <ydwe/config.h>

namespace ydwe { namespace util {
	YDWE_BASE_API std::wstring u2w(std::string  const& from);
	YDWE_BASE_API std::string  w2u(std::wstring const& from);
	YDWE_BASE_API std::wstring a2w(std::string  const& from);
	YDWE_BASE_API std::string  w2a(std::wstring const& from);
	YDWE_BASE_API std::string  u2a(std::string  const& from);
	YDWE_BASE_API std::string  a2u(std::string  const& from);
	YDWE_BASE_API bool is_utf8(const char *source);
}}
