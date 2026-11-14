#include "Core/FileSystem.h"

#include "RenderBackend/RenderBackendDef.h"

const std::string File::projectDir = WORKDIR;
const std::string File::resourcesDir = File::projectDir + "Resources/";
const std::string File::shadersDir = File::projectDir + "Shaders/" + RBACKEND_NAME + "/";