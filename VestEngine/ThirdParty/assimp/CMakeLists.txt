include(FetchContent)
FetchContent_Declare(
	assimp
	GIT_REPOSITORY https://github.com/assimp/assimp.git
	GIT_TAG v6.0.5
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(assimp)

set_target_properties(assimp 
	zlibstatic 
	unit 
	UpdateAssimpLibsDebugSymbolsAndDLLs 
	PROPERTIES FOLDER "ThirdParty/assimp")