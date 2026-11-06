#pragma once

#include <type_traits>

#define RESOURCE_INVALID_ID UINT32_MAX

// todo : introduce ref counting ?
struct ResourceHandle
{
	uint32_t handle;

	ResourceHandle(uint32_t inHandle)
		: handle(inHandle)
	{}

	bool isValid() const
	{
		return handle != RESOURCE_INVALID_ID;
	}

	bool operator!=(const ResourceHandle& inOther) const
	{
		return handle != inOther.handle;
	}

	bool operator==(const ResourceHandle& inOther) const
	{
		return handle == inOther.handle;
	}

	static ResourceHandle invalid;
};

template<>
struct std::hash<ResourceHandle>
{
	size_t operator()(const ResourceHandle& inResource) const
	{
		return std::hash<uint32_t>{}(inResource.handle);
	}
};