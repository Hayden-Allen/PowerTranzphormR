#include "pch.h"
#include "preview_window.h"
#include "app_ctx.h"
#include "preview_layer.h"
#include "core/sgnode.h"
#include "core/smnode.h"

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
	if (!m_app_ctx->mgl_ctx.is_cursor_locked())
	{
		sgnode* const sg = m_app_ctx->get_selected_sgnode();
		smnode* const sm = m_app_ctx->get_selected_static_mesh();
		light* const sl = m_app_ctx->get_selected_light();
		waypoint* const sw = m_app_ctx->get_selected_waypoint();
		if (sg || sm || sl || sw)
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

			const tmat<space::WORLD, space::CAMERA>& view = m_app_ctx->preview_cam.get_view();
			const pmat<space::CAMERA, space::CLIP>& proj = m_app_ctx->preview_cam.get_proj();
			if (sg)
			{
				// make a copy of node's transform so that the gizmo can modify it
				tmat<space::OBJECT, space::PARENT> current_mat = sg->get_mat();
				ImGuizmo::Manipulate((view * sg->accumulate_parent_mats()).e, proj.e, m_app_ctx->gizmo_op, ImGuizmo::LOCAL, current_mat.e);

				if (ImGuizmo::IsUsing())
				{
					// if this is the first frame it's being used, save initial transform
					if (!m_was_using_imguizmo)
					{
						m_imguizmo_undo_mat = sg->get_mat();
					}
					m_was_using_imguizmo = true;
					// propagate gizmo's changes to node
					if (current_mat != sg->get_mat())
						sg->set_transform(current_mat);
				}
				// ImGuizmo not being used this frame, but was last frame
				else if (m_was_using_imguizmo)
				{
					m_was_using_imguizmo = false;
					// push result of ImGuizmo onto action stack
					if (sg)
					{
						m_app_ctx->transform_action(sg, m_imguizmo_undo_mat, sg->get_mat());
					}
				}
			}
			else if (sm)
			{
				// make a copy of node's transform so that the gizmo can modify it
				tmat<space::OBJECT, space::WORLD> current_mat = sm->get_mat();
				ImGuizmo::Manipulate(view.e, proj.e, m_app_ctx->gizmo_op, ImGuizmo::LOCAL, current_mat.e);

				if (ImGuizmo::IsUsing())
				{
					m_was_using_imguizmo = true;
					// propagate gizmo's changes to node
					if (current_mat != sm->get_mat())
						sm->set_transform(current_mat);
				}
				// ImGuizmo not being used this frame, but was last frame
				else if (m_was_using_imguizmo)
				{
					m_was_using_imguizmo = false;
				}
			}
			else if (sl)
			{
				// make a copy of node's transform so that the gizmo can modify it
				tmat<space::OBJECT, space::WORLD> current_mat = sl->get_mat();
				current_mat.t[3] = 1.f;
				ImGuizmo::Manipulate(view.e, proj.e, m_app_ctx->gizmo_op, ImGuizmo::LOCAL, current_mat.e);

				if (ImGuizmo::IsUsing())
				{
					m_was_using_imguizmo = true;
					// propagate gizmo's changes to node
					current_mat.t[3] = (f32)sl->get_type();
					if (current_mat != sl->get_mat())
						sl->set_mat(current_mat);
					m_app_ctx->scene.update_light(sl);
				}
				// ImGuizmo not being used this frame, but was last frame
				else if (m_was_using_imguizmo)
				{
					m_was_using_imguizmo = false;
				}
			}
			else if (sw)
			{
				// make a copy of node's transform so that the gizmo can modify it
				tmat<space::OBJECT, space::WORLD> current_mat = sw->get_mat();
				ImGuizmo::Manipulate(view.e, proj.e, m_app_ctx->gizmo_op, ImGuizmo::LOCAL, current_mat.e);

				if (ImGuizmo::IsUsing())
				{
					m_was_using_imguizmo = true;
					// propagate gizmo's changes to node
					if (current_mat != sw->get_mat())
						sw->set_mat(current_mat);
				}
				// ImGuizmo not being used this frame, but was last frame
				else if (m_was_using_imguizmo)
				{
					m_was_using_imguizmo = false;
				}
			}
			ImGui::PopClipRect();
		}
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
