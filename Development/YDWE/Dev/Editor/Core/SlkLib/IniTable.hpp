#pragma once

#include "BaseTable.hpp"

namespace slk
{
	class IniSingle : public HashTable<std::string, std::string>::Type
	{
	};

	class IniTable : public HashTable<std::string, IniSingle>::Type
	{
	};
}
