set(SPIRV_REFLECT_EXECUTABLE OFF)
set(SPIRV_REFLECT_STATIC_LIB ON)
set(SPIRV_REFLECT_INSTALL OFF)

include(FetchContent)
FetchContent_Declare(
	spirv_reflect
	GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Reflect
	GIT_TAG vulkan-sdk-1.4.363.0
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(spirv_reflect)

target_include_directories(spirv-reflect-static PUBLIC ${spirv_reflect_SOURCE_DIR})
message(STATUS "spirv_reflect_SOURCE_DIR: ${spirv_reflect_SOURCE_DIR}")

set_target_properties(spirv-reflect-static PROPERTIES FOLDER "ThirdParty")

# todo: add this, need to add vulkan dependency
# target_compile_definitions(spirv-reflect-static PUBLIC SPIRV_REFLECT_USE_SYSTEM_SPIRV_H)