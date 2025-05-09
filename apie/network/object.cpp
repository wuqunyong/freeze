#include "apie/network/object.hpp"

#include <string.h>
#include <stdarg.h>



namespace apie {
	object_t::object_t(uint32_t tid) :
		tid_(tid)
	{
	}

	object_t::~object_t()
	{

	}

	uint32_t object_t::get_tid()
	{
		return tid_;
	}
}
