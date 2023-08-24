#include "pch.h"
#include "preview_window.h"

preview_window::preview_window(const mgl::context& mgl_context, preview_layer* const pl, imgui_layer* const il) :
	imgui_window("Preview"),
	m_mgl_context(mgl_context),
	m_preview_layer(pl),
	m_imgui_layer(il)
{}



void preview_window::handle_frame()
{
	// GGTODO comment
	const auto& fb = m_preview_layer->get_framebuffer();
	const ImVec2 win_min = ImGui::GetWindowContentRegionMin(), win_max = ImGui::GetWindowContentRegionMax();
	const f32 win_w = win_max.x - win_min.x, win_h = win_max.y - win_min.y;
	const f32 fb_w = fb.get_width<f32>(), fb_h = fb.get_height<f32>();
	const f32 xr = win_w / fb_w, yr = win_h / fb_h;
	const f32 r = std::min(xr, yr);
	const f32 ox = (win_w - fb_w * r) * 0.5f, oy = (win_h - fb_h * r) * 0.5f;
	const ImVec2 img_pos(ox + win_min.x, oy + win_min.y);
	ImGui::SetCursorPos(img_pos);
	const ImVec2 img_dim(fb_w * r, fb_h * r);
	ImGui::Image(fb.get_imgui_color_id(), img_dim, ImVec2(0, 1), ImVec2(1, 0));

	// if something in the scene graph is selected, show a transform gizmo for it
	sgnode* target = m_preview_layer->get_scene()->get_selected_node();
	if (target)
	{
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		// TODO make this the size of the framebuffer?
		const auto& win_pos = ImGui::GetWindowPos();
		ImGuizmo::SetRect(win_pos.x, win_pos.y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

		// make a copy of node's transform so that the gizmo can modify it
		tmat<space::OBJECT, space::WORLD> current_mat = target->mat;
		const tmat<space::WORLD, space::CAMERA>& view = m_preview_layer->get_camera().get_view();
		const pmat<space::CAMERA, space::CLIP>& proj = m_preview_layer->get_camera().get_proj();
		ImGuizmo::Manipulate(view.e, proj.e, ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::WORLD, current_mat.e);

		// whether or not ImGuizmo was being used last frame
		static bool was_using = false;
		static tmat<space::OBJECT, space::WORLD> initial_mat;
		// if using ImGuizmo this frame
		if (ImGuizmo::IsUsing())
		{
			// if this is the first frame it's being used, save initial transform
			if (!was_using)
			{
				initial_mat = target->mat;
			}
			was_using = true;
			// propagate gizmo's changes to node
			target->set_transform(current_mat);
		}
		// ImGuizmo not being used this frame, but was last frame
		else if (was_using)
		{
			was_using = false;
			// push result of ImGuizmo onto action stack
			if (target)
			{
				m_imgui_layer->transform_action(target, initial_mat, current_mat);
			}
		}
	}
	if (!ImGuizmo::IsUsing() && ImGui::IsItemClicked(GLFW_MOUSE_BUTTON_LEFT))
	{
		ImGui_ImplGlfw_MouseButtonCallback(m_mgl_context.window, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0);
		m_preview_layer->enable();
	}
}
