#pragma once

#include <type_traits>

#define RESOURCE_INVALID UINT32_MAX

// todo : introduce ref counting ?
struct ResourceHandle
{
	uint32_t handle;

	ResourceHandle()
		: ResourceHandle(RESOURCE_INVALID)
	{
	}

	ResourceHandle(uint32_t inHandle)
		: handle(inHandle)
	{
	}

	bool IsValid() const
	{
		return handle != RESOURCE_INVALID;
	}

	bool operator==(const ResourceHandle& inOther) const
	{
		return handle == inOther.handle;
	}
};

template<>
struct std::hash<ResourceHandle>
{
	size_t operator()(const ResourceHandle& inResource) const
	{
		return std::hash<uint32_t>{}(inResource.handle);
	}
};

