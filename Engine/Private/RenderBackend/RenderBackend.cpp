#include "RenderBackend/RenderBackend.h"

#ifdef RENDER_OPENGL
#include "RenderBackend_OpenGL/RenderBackend_OpenGL.inl"
#else
#error Graphic API not supported
#endif