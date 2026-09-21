cmake_minimum_required(VERSION 3.25...4.4)
project(VestEngine VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =====
# LIBRARIES
# =====

add_subdirectory("ThirdParty/glfw-3.4")
add_subdirectory("ThirdParty/glad")
add_subdirectory("ThirdParty/glm")
add_subdirectory("ThirdParty/stb")
add_subdirectory("ThirdParty/imgui")
add_subdirectory("ThirdParty/assimp")
option(TRACY_ENABLE "" ON )
option(TRACY_ON_DEMAND "" ON )
add_subdirectory("ThirdParty/tracy")
# =====
# ENGINE TARGET - STATIC LIB
# =====

file(GLOB_RECURSE ENGINE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Private/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Public/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Engine/Public/*.inl")
	
add_library(Engine STATIC ${ENGINE_FILES})
target_include_directories(Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Public")
target_link_libraries(Engine PRIVATE 	glfw)
target_link_libraries(Engine PRIVATE 	glad)
target_link_libraries(Engine PRIVATE 	stb)
target_link_libraries(Engine PRIVATE 	assimp)
target_link_libraries(Engine PUBLIC 	glm)
target_link_libraries(Engine PUBLIC 	imgui)
target_link_libraries(Engine PUBLIC		Tracy::TracyClient)

option(TRACY_ENABLE "" ON)
option(TRACY_ON_DEMAND "" ON)

#add_custom_command(TARGET Engine POST_BUILD
#	COMMAND ${CMAKE_COMMAND} -E copy_if_different
#	"${CMAKE_SOURCE_DIR}/ThirdParty/assimp/bin/assimp-vc143-mtd.dll"
#    	"$<TARGET_FILE_DIR:Engine>"
#	VERBATIM)

# =====
# UNIT TEST TARGET - STATIC LIB
# =====

file(GLOB_RECURSE UNITTEST_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/UnitTest/Private/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/UnitTest/Private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/UnitTest/Public/*.h")
	
add_library(UnitTest STATIC ${UNITTEST_FILES})

target_include_directories(UnitTest PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/UnitTest/Public")
target_include_directories(UnitTest PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/UnitTest/Private")
target_link_libraries(UnitTest PRIVATE Engine)

# =====
# EDITOR TARGET - EXECUTABLE
# =====

file(GLOB_RECURSE EDITOR_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Editor/Private/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Editor/Public/*.h")

add_executable(Editor ${EDITOR_FILES})

target_include_directories(Editor PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Editor/Public")
target_link_libraries(Editor PRIVATE Engine)
target_link_libraries(Editor PRIVATE UnitTest)

# =====
# DEFINES
# =====

target_compile_definitions(Engine PUBLIC WORKDIR="${CMAKE_SOURCE_DIR}")

# =====
# COMPILATION
# =====

target_compile_options(Engine PRIVATE /W3 /permissive- /Zc:preprocessor /MP)
target_compile_options(Editor PRIVATE /W3 /permissive- /Zc:preprocessor /MP)
target_compile_options(UnitTest PRIVATE /W3 /permissive- /Zc:preprocessor /MP)

# =====
# VISUAL STUDIO
# =====

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${ENGINE_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${EDITOR_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${UNITTEST_FILES})
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Editor)