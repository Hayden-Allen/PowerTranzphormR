#include "pch.h"
#include "materials_list_window.h"
#include "scene_material.h"

materials_list_window::materials_list_window(app_ctx* const a_ctx) :
	imgui_window(a_ctx, "Materials")
{
	//
}

void materials_list_window::handle_frame()
{
	const auto& mtls = m_app_ctx->scene.get_materials();
	for (auto it = mtls.begin(); it != mtls.end(); ++it)
	{
		if (it == mtls.begin())
		{
			ImGui::TextDisabled(it->second->name.c_str());
			continue;
		}

		bool selected = m_app_ctx->scene.get_selected_material() == it->second;
		ImGui::Selectable(it->second->name.c_str(), &selected);
		if (selected)
		{
			m_app_ctx->scene.set_selected_material(it->second);
		}
	}
}
