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

	const auto& sorted_mtls = m_app_ctx->get_sorted_materials();

	for (size_t i = 0; i < sorted_mtls.size(); ++i)
	{
		const u32 id = sorted_mtls[i].first;
		scene_material* const mtl = sorted_mtls[i].second;
		if (i == 0)
		{
			ImGui::TextDisabled(mtl->name.c_str());
			continue;
		}

		bool selected = m_app_ctx->get_selected_material() == mtl;
		ImGui::PushID(std::to_string(id).c_str());
		ImGui::Selectable(mtl->name.c_str(), &selected);
		ImGui::PopID();
		scene_material* needs_select = m_app_ctx->get_imgui_needs_select_unfocused_mtl();
		if (needs_select)
		{
			if (mtl == needs_select)
			{
				ImGui::SetKeyboardFocusHere(-1);
			}
		}
		else if (ImGui::IsItemFocused())
		{
			m_app_ctx->set_selected_material(mtl);
		}
	}

	m_app_ctx->unset_imgui_needs_select_unfocused_mtl();
}
