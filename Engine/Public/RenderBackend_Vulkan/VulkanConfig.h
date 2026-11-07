#pragma once

#define KB 1024
#define MB (KB * KB)

namespace renderConfig
{
	static constexpr uint8_t getFrameCount() { return 2; }
	static constexpr size_t getFrameBufferSize() { return 10 * MB; }
	static constexpr uint32_t getMaterialBufferSize() { return KB; }
	static constexpr uint8_t getTextureArraySize() { return 16; }
}