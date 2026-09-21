include(FetchContent)
FetchContent_Declare(
	glm
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	GIT_TAG 1.0.3
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(glm)

set_target_properties(glm PROPERTIES FOLDER "ThirdParty")