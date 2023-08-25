#include "pch.h"
#include "preview_window.h"

preview_window::preview_window(app_ctx* const a_ctx) :
	imgui_window(a_ctx, "Preview")
{}



void preview_window::handle_frame()
{
	// GGTODO comment
	const auto& fb = m_app_ctx->preview_fb;
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

	// if something in the scene graph is selected, and the cursor is not locked, then show a transform gizmo for it
	sgnode* target = m_app_ctx->scene.get_selected_node();
	if (target && !m_app_ctx->mgl_ctx.is_cursor_locked())
	{
		const auto& win_pos = ImGui::GetWindowPos();
		auto clip_min = win_pos;
		clip_min.x += img_pos.x;
		clip_min.y += img_pos.y;
		auto clip_max = clip_min;
		clip_max.x += img_dim.x;
		clip_max.y += img_dim.y;
		ImGui::PushClipRect(clip_min, clip_max, false);

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(clip_min.x, clip_min.y, img_dim.x, img_dim.y);

		// make a copy of node's transform so that the gizmo can modify it
		tmat<space::OBJECT, space::WORLD> current_mat = target->mat;
		const tmat<space::WORLD, space::CAMERA>& view = m_app_ctx->preview_cam.get_view();
		const pmat<space::CAMERA, space::CLIP>& proj = m_app_ctx->preview_cam.get_proj();
		ImGuizmo::Manipulate(view.e, proj.e, m_app_ctx->gizmo_op, ImGuizmo::LOCAL, current_mat.e);

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
				m_app_ctx->transform_action(target, initial_mat, current_mat);
			}
		}

		ImGui::PopClipRect();
	}
	if (!ImGuizmo::IsUsing() && ImGui::IsItemClicked(GLFW_MOUSE_BUTTON_LEFT))
	{
		ImGui_ImplGlfw_MouseButtonCallback(m_app_ctx->mgl_ctx.window, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0);
		m_enable_callback();
	}
}

void preview_window::set_enable_callback(const std::function<void()>& callback)
{
	m_enable_callback = callback;
}
