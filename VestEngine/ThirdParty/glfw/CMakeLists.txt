include(FetchContent)
FetchContent_Declare(
	glfw
	GIT_REPOSITORY https://github.com/glfw/glfw.git
	GIT_TAG 3.5.1
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(glfw)

set_target_properties(glfw 
	uninstall 
	update_mappings 
	PROPERTIES FOLDER "ThirdParty/glfw")