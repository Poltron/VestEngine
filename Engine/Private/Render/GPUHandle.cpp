#include "Render/GPUHandle.h"

GPUAllocHandle GPUAllocHandle::invalid = GPUAllocHandle(GPURESOURCE_INVALID_ID);
BufferHandle BufferHandle::invalid = BufferHandle(GPURESOURCE_INVALID_ID, 0, 0);