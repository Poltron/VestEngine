#include "Widgets/RendererInfo.h"

#include "imgui.h"

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

		std::string drawCallsTotal = "Draw calls: ";
		drawCallsTotal += std::to_string(render::getRenderer()->getFrameInfo().drawCalls);
		ImGui::Text(drawCallsTotal.c_str());

		ImGui::End();
	}
}