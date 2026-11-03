#pragma once

namespace renderConfig
{
	static constexpr uint8_t getFrameCount() { return 2; }
	static constexpr size_t getFrameBufferSize() { return 10240; }
	static constexpr uint32_t getMaterialBufferSize() { return 10240; }
	static constexpr uint8_t getTextureArraySize() { return 16; }
}