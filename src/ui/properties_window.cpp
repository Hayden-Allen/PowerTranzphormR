#include "pch.h"
#include "properties_window.h"
#include "geom/generated_mesh.h"
#include "app_ctx.h"
#include "scene_graph.h"

properties_window::properties_window(app_ctx* const a_ctx) :
	imgui_window(a_ctx, "Properties")
{
}

void properties_window::handle_frame()
{
	sgnode* const selected = m_app_ctx->get_selected_sgnode();
	if (!selected)
	{
		return;
	}

	if (selected->is_root())
	{
		handle_snapping_angle();
	}
	handle_transform(selected);
	if (selected->is_mesh())
	{
		handle_mesh(selected);
	}
}

void properties_window::handle_snapping_angle()
{
	const bool start_all = scene_ctx::s_snap_all;
	ImGui::Checkbox("Snap All", &scene_ctx::s_snap_all);
	const f32 start_angle = scene_ctx::s_snap_angle;
	ImGui::SliderAngle("Snap Angle", &scene_ctx::s_snap_angle, 0.f, 360.f);

	if (start_angle != scene_ctx::s_snap_angle || start_all != scene_ctx::s_snap_all)
		m_app_ctx->scene.get_sg_root()->set_dirty();
}

void properties_window::handle_transform(sgnode* const selected)
{
	bool dirty = false;
	float trans[3] = { 0.f }, rot[3] = { 0.f }, scale[3] = { 0.f };
	ImGuizmo::DecomposeMatrixToComponents(selected->mat.e, trans, rot, scale);
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
		ImGuizmo::RecomposeMatrixFromComponents(trans, rot, scale, selected->mat.e);
		selected->set_transform(selected->mat.e);
	}
}

void properties_window::handle_mesh(sgnode* const selected)
{
	for (const auto& prop : selected->gen->get_params())
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
