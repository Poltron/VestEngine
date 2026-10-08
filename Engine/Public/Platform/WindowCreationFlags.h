#pragma once

#include <stdint.h>

namespace contextHints
{
	enum Requirements : uint8_t
	{
		ClientAPI = 1 << 0,
		ContextCreationAPI = 1 << 1,
		MajorVersion = 1 << 2,
		MinorVersion = 1 << 3,
		OpenGLProfile = 1 << 4
	};

	enum class EClientAPI : uint8_t
	{
		NONE = 0,
		OPENGL,
		OPENGLES
	};
	
	enum class EContextCreationAPI : uint8_t
	{
		NATIVE = 0,
		EGL,
		OSMESA
	};
	
	enum class EOpenGLProfile : uint8_t
	{
		ANY = 0,
		COMPAT,
		CORE
	};
}

namespace windowHints
{
	enum EWindowFlags : uint8_t
	{
		RESIZABLE = 1 << 0,
		VISIBLE = 1 << 1,
		DECORATED = 1 << 2,
		FOCUSED = 1 << 3,
		AUTO_ICONIFY = 1 << 4,
		FLOATING = 1 << 5,
		MAXIMIZED = 1 << 6,
		CENTER_CURSOR = 1 << 7
	};

	enum class WindowFlagValue : uint8_t
	{
		FALSE = 0,
		TRUE = 1
	};
}