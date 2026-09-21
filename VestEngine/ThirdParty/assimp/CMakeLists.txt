include(FetchContent)
FetchContent_Declare(
	assimp
	GIT_REPOSITORY https://github.com/assimp/assimp.git
	GIT_TAG v6.0.5
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(assimp)