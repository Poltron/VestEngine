#include "Widgets/Components/RigidbodyComponentInspector.h"

#include "imgui.h"

#include "Core/Engine.h"
#include "Core/Scene.h"

std::string RigidbodyComponentInspector::getLabel()
{
	return "Rigidbody";
}

void RigidbodyComponentInspector::update()
{
	RigidbodyComponent* rigidbody = engine::getScene()->rigidbodyComponents.get(entity);
	if (!rigidbody)
	{
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "Can't retrieve data");
		return;
	}

	angularVelocity = rigidbody->angularVelocity;
	linearVelocity = rigidbody->linearVelocity;

	if (ImGui::InputFloat4("Angular Velocity", &angularVelocity.x))
	{
		rigidbody->angularVelocity = angularVelocity;
	}

	if (ImGui::InputFloat4("Linear Velocity", &linearVelocity.x))
	{
		rigidbody->linearVelocity = linearVelocity;
	}
}