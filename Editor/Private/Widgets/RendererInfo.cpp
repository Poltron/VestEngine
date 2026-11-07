#include "Widgets/RendererInfo.h"

#include <imgui.h>
#include <string>

#include "Render/Renderer.h"

void RendererInfo::initialize()
{}

void RendererInfo::update()
{
	if (ImGui::Begin("Renderer Info", 0, ImGuiWindowFlags_NoResize))
	{
		ImGui::SetWindowSize({ 200, 200 });

		std::string renderedMeshTotal = "Rendered mesh: ";
		renderedMeshTotal += std::to_string(render::getRenderer()->getFrameInfo().renderedMeshTotal);
		ImGui::Text(renderedMeshTotal.c_str());

		std::string culledMeshTotal = "Culled meshes: ";
		culledMeshTotal += std::to_string(render::getRenderer()->getFrameInfo().culledMeshTotal);
		ImGui::Text(culledMeshTotal.c_str());

		std::string debugShapesTotal = "Debug shapes: ";
		debugShapesTotal += std::to_string(render::getRenderer()->getFrameInfo().debugShapesTotal);
		ImGui::Text(debugShapesTotal.c_str());

		ImGui::Text(" --- ");

		std::string drawIndexedTotal = "Draw: ";
		drawIndexedTotal += std::to_string(render::getRenderer()->getFrameInfo().drawIndexed);
		ImGui::Text(drawIndexedTotal.c_str());

		std::string bindMatTotal = "Bind mat: ";
		bindMatTotal += std::to_string(render::getRenderer()->getFrameInfo().bindMat);
		ImGui::Text(bindMatTotal.c_str());

		ImGui::End();
	}
}