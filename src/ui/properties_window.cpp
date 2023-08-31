#include "pch.h"
#include "properties_window.h"
#include "geom/generated_mesh.h"
#include "app_ctx.h"
#include "sgnode.h"
#include "scene_material.h"

properties_window::properties_window(app_ctx* const a_ctx) :
	imgui_window(a_ctx, "Properties")
{
}

void properties_window::handle_frame()
{
	sgnode* const selected_sgnode = m_app_ctx->get_selected_sgnode();
	if (selected_sgnode)
	{
		handle_sgnode_frame(selected_sgnode);
	}

	scene_material* const selected_material = m_app_ctx->get_selected_material();
	if (selected_material)
	{
		handle_material_frame(selected_material);
	}
}

void properties_window::handle_sgnode_frame(sgnode* const selected)
{
	if (selected->is_root())
	{
		handle_sgnode_snapping_angle();
	}
	handle_sgnode_transform(selected);
	if (!selected->is_operation())
	{
		handle_sgnode_mesh(selected);
	}
}

void properties_window::handle_sgnode_snapping_angle()
{
	const bool start_all = scene_ctx::s_snap_all;
	ImGui::Checkbox("Snap All", &scene_ctx::s_snap_all);
	const f32 start_angle = scene_ctx::s_snap_angle;
	ImGui::SliderAngle("Snap Angle", &scene_ctx::s_snap_angle, 0.f, 360.f);

	if (start_angle != scene_ctx::s_snap_angle || start_all != scene_ctx::s_snap_all)
		m_app_ctx->scene.get_sg_root()->set_dirty();
}

void properties_window::handle_sgnode_transform(sgnode* const selected)
{
	bool dirty = false;
	float trans[3] = { 0.f }, rot[3] = { 0.f }, scale[3] = { 0.f };
	ImGuizmo::DecomposeMatrixToComponents(selected->get_mat().e, trans, rot, scale);
	if (ImGui::DragFloat3("Position", trans, 0.01f))
	{
		dirty = true;
	}
	if (ImGui::DragFloat3("Rotation", rot, 0.01f))
	{
		dirty = true;
	}
	if (ImGui::DragFloat3("Scale", scale, 0.01f))
	{
		dirty = true;
	}
	if (dirty)
	{
		ImGuizmo::RecomposeMatrixFromComponents(trans, rot, scale, selected->get_mat().e);
		selected->set_transform(selected->get_mat().e);
	}
}

void properties_window::handle_sgnode_mesh(sgnode* const selected)
{
	if (!selected->is_operation() && !selected->is_frozen())
	{
		u32 combo_selected_mtl_id = selected->get_material();
		if (ImGui::BeginCombo("Material", m_app_ctx->scene.get_material(combo_selected_mtl_id)->name.c_str()))
		{
			const auto& sorted_mtls = m_app_ctx->get_sorted_materials();
			for (const auto& pair : sorted_mtls)
			{
				bool cur_mtl_combo_selected = pair.first == combo_selected_mtl_id;
				ImGui::PushID(pair.first);
				if (ImGui::Selectable(pair.second->name.c_str(), cur_mtl_combo_selected))
				{
					m_app_ctx->set_material(selected, pair.first);
				}
				if (cur_mtl_combo_selected)
				{
					ImGui::SetItemDefaultFocus();
				}
				ImGui::PopID();
			}
			ImGui::EndCombo();
		}
	}

	for (const auto& prop : selected->get_gen()->get_params())
	{
		if (prop.second.is_float)
		{
			if (ImGui::DragFloat(prop.first.c_str(), static_cast<f32*>(prop.second.value), prop.second.speed, prop.second.min, prop.second.max))
			{
				selected->set_gen_dirty();
			}
		}
		else
		{
			if (ImGui::DragInt(prop.first.c_str(), static_cast<s32*>(prop.second.value), prop.second.speed, static_cast<s32>(prop.second.min), static_cast<s32>(prop.second.max)))
			{
				selected->set_gen_dirty();
			}
		}
	}
}

void properties_window::handle_material_frame(scene_material* const selected)
{
	selected->for_each_texture([&](const std::string& name, const mgl::texture2d_rgb_u8* tex_DONOTUSE)
	{
		handle_material_texture(selected, name);
	});
}

void properties_window::handle_material_texture(scene_material* const selected_mtl, const std::string& name)
{
	const ImVec2 win_min = ImGui::GetWindowContentRegionMin(), win_max = ImGui::GetWindowContentRegionMax();
	const f32 win_w = win_max.x - win_min.x;
	const f32 img_w = std::min(win_w, m_app_ctx->mgl_ctx.get_height() * 0.3f);
	const ImVec2 img_dim(img_w, img_w);
	const f32 btn_w = std::min(win_w, 100.0f);
	ImGui::SetCursorPosX(win_min.x + (win_w - btn_w) * 0.5f);
	if (ImGui::Button("Reset", ImVec2(btn_w, 0)))
	{
		selected_mtl->set_texture(name, "<NULL>");
	}
	ImGui::SetCursorPosX(win_min.x + (win_w - img_dim.x) * 0.5f);
	if (ImGui::ImageButton(name.c_str(), selected_mtl->get_texture(name)->get_imgui_id(), img_dim, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f))) {
		nfdchar_t* nfd_path = nullptr;
		nfdfilteritem_t nfd_filters[1] = { { "Image file", "png,jpg,bmp" } };
		nfdresult_t nfd_res = NFD_OpenDialog(&nfd_path, nfd_filters, 1, nullptr);
		if (nfd_res == NFD_OKAY)
		{
			selected_mtl->set_texture(name, nfd_path);
			NFD_FreePath(nfd_path);
		}
	}
}
