#include "pch.h"
#include "materials_list_window.h"
#include "core/scene_material.h"
#include "app_ctx.h"

materials_list_window::materials_list_window(app_ctx* const a_ctx) :
	imgui_window(a_ctx, "Materials")
{
	//
}



void materials_list_window::handle_focused(bool focused)
{
	if (focused)
	{
		if (!m_was_focused)
		{
			m_app_ctx->set_sel_type(global_selection_type::material);
		}
		m_was_focused = true;
	}
	else
	{
		m_was_focused = false;
	}
}
void materials_list_window::handle_frame()
{
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
		if (mtl == m_renaming)
		{
			const f32 x = ImGui::GetCursorPosX();
			const f32 y = ImGui::GetCursorPosY();

			constexpr u32 BUF_SIZE = 32;
			char buf[32] = { 0 };
			memcpy_s(buf, BUF_SIZE, mtl->name.c_str(), mtl->name.size());

			// hacky, but works
			ImGui::SetCursorPosX(x - ImGui::GetStyle().FramePadding.x / 2 - 2);
			ImGui::SetCursorPosY(y - ImGui::GetStyle().FramePadding.y / 2 - 1);
			if (ImGui::InputText("##MW_RENAME", buf, BUF_SIZE, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				std::string new_name(buf);
				if (!new_name.empty())
				{
					mtl->name = new_name;
				}
				m_renaming = nullptr;
			}

			if (m_rename_needs_focus)
			{
				ImGui::SetKeyboardFocusHere(-1);
				m_rename_needs_focus = false;
			}
			else if (!ImGui::IsItemActive())
			{
				m_renaming = nullptr;
			}
		}
		else
		{
			ImGui::Selectable(mtl->name.c_str(), &selected);
		}
		if (ImGui::BeginPopupContextItem())
		{
			m_app_ctx->set_selected_material(mtl);
			if (ImGui::MenuItem("Rename"))
			{
				set_renaming(mtl);
			}
			ImGui::EndPopup();
		}
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
void materials_list_window::set_renaming(scene_material* mtl)
{
	assert(!m_renaming);
	m_renaming = mtl;
	m_rename_needs_focus = true;
}
const scene_material* materials_list_window::get_renaming() const
{
	return m_renaming;
}
