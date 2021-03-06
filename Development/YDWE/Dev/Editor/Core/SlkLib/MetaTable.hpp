#pragma once

#include <string>
#include <set>
#include "ObjectId.hpp"
#include "BaseTable.hpp"
#include "SlkTable.hpp"
#include "VariableData.hpp"

namespace slk
{
	struct MetaSingle
	{
		VariableData::Type type;
		uint32_t           index;
		uint32_t           stringExt;
		std::string        field;
		std::string        slk;
	};

	class SLKLIB_API MetaTable : public BaseTable<MetaTable, MetaSingle>
	{
	};
}
