#include "pch.h"
#include "materials_list_window.h"
#include "scene_material.h"
#include "app_ctx.h"

materials_list_window::materials_list_window(app_ctx* const a_ctx) :
	imgui_window(a_ctx, "Materials")
{
	//
}

void materials_list_window::handle_frame()
{
	if (ImGui::IsWindowFocused())
	{
		m_app_ctx->sel_type = global_selection_type::material;
	}

	const auto& unordered_mtls = m_app_ctx->scene.get_materials();
	std::vector<scene_material*> sorted_mtls;
	for (const auto& i : unordered_mtls)
	{
		if (i.first != 0)
		{
			sorted_mtls.emplace_back(i.second);
		}
	}
	std::sort(sorted_mtls.begin(), sorted_mtls.end(), [](scene_material* a, scene_material* b) {
		return a->name < b->name;
	});
	sorted_mtls.emplace(sorted_mtls.begin(), unordered_mtls.at(0));

	for (size_t i = 0; i < sorted_mtls.size(); ++i)
	{
		const auto& mtl = sorted_mtls[i];
		if (i == 0)
		{
			ImGui::TextDisabled(mtl->name.c_str());
			continue;
		}

		bool selected = m_app_ctx->get_selected_material() == mtl;
		ImGui::Selectable(mtl->name.c_str(), &selected);
		scene_material* needs_select = m_app_ctx->get_imgui_needs_select_unfocused_mtl();
		if (needs_select)
		{
			if (mtl == needs_select)
			{
				ImGui::SetKeyboardFocusHere(-1);
			}
		}
		else if (selected || ImGui::IsItemFocused())
		{
			m_app_ctx->set_selected_material(mtl);
		}
	}
}
