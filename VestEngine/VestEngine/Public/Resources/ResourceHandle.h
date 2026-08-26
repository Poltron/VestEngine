#pragma once

#include <functional>

struct ResourceHandle
{
	uint32_t handle;

	ResourceHandle()
		: ResourceHandle(0)
	{
	}

	ResourceHandle(uint32_t inHandle)
		: handle(inHandle)
	{
	}

	bool IsValid() const
	{
		return handle != 0;
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

