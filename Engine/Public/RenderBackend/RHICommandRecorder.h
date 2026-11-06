#pragma once
#include "Core/Allocators/LinearAllocator.h"

enum class ECommandType : uint32_t
{
	StartRenderpass,
	EndRenderpass,
	BindMaterial,
	DrawIndexed
};

struct CommandHeader
{
	uint32_t size;
	ECommandType type;
};

struct StartRenderpassCommand
{
	uint8_t passId;
};

struct EndRenderpassCommand
{
	uint8_t passId;
};

struct BindMaterialCommand
{
	GPUProxyHandle material = GPUProxyHandle::invalid;;
};

struct DrawIndexCommand
{
	GPUProxyHandle mesh = GPUProxyHandle::invalid;
	GPUProxyHandle material = GPUProxyHandle::invalid;
	glm::mat4 model;
};

class RHICommandRecorder final
{
	LinearAllocator commands;

public:
	const LinearAllocator& getCommands() const { return commands; }

	RHICommandRecorder()
	{
		commands.reserve(1024 * 1024);
	}

	~RHICommandRecorder() = default;

	RHICommandRecorder(const RHICommandRecorder& inOther) = delete;
	RHICommandRecorder& operator=(const RHICommandRecorder& inOther) = delete;

	RHICommandRecorder(RHICommandRecorder&& inOther) = delete;
	RHICommandRecorder& operator=(RHICommandRecorder&& inOther) = delete;

	void add(const CommandHeader* header, const void* inData)
	{
		constexpr size_t headerSize = sizeof(CommandHeader);
		size_t offset = commands.create(headerSize + header->size);
		uint8_t* ptr = commands.get(offset);
		memcpy(ptr, header, headerSize);
		memcpy(ptr + headerSize, inData, header->size);
	}

	void reset()
	{
		commands.clear();
	}
};