#pragma once

#include <type_traits>

#define INVALID_RESOURCE_HANDLE UINT32_MAX

// todo : introduce ref counting ?
struct ResourceHandle
{
	uint32_t handle;

	ResourceHandle()
		: ResourceHandle(INVALID_RESOURCE_HANDLE)
	{
	}

	ResourceHandle(uint32_t inHandle)
		: handle(inHandle)
	{
	}

	bool IsValid() const
	{
		return handle != INVALID_RESOURCE_HANDLE;
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

