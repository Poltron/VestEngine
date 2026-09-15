add_library(glfw STATIC IMPORTED GLOBAL)
set_target_properties(glfw PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES 	"${CMAKE_SOURCE_DIR}/ThirdParty/glfw-3.4/includes"
    IMPORTED_LOCATION 				"${CMAKE_SOURCE_DIR}/ThirdParty/glfw-3.4/lib/glfw3.lib"
	IMPORTED_LOCATION_DEBUG 		"${CMAKE_SOURCE_DIR}/ThirdParty/glfw-3.4/lib/glfw3.lib"
	MAP_IMPORTED_CONFIG_DEBUG 		Debug)