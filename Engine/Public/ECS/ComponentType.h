#pragma once

struct ComponentTypeID
{
	template<typename T>
	static inline size_t GetTypeID()
	{
		static const size_t id = ComponentTypeID::GetID();
		return id;
	}

private:
	static size_t GetID()
	{
		nextID++;
		return nextID;
	}

	static size_t nextID;
};