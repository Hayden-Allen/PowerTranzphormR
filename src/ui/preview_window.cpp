#include "pch.h"
#include "preview_window.h"

preview_window::preview_window(const mgl::context& mgl_context, preview_layer* const layer) :
	m_mgl_context(mgl_context),
	m_layer(layer)
{
	title = "Preview";
}



void preview_window::handle_frame()
{
	// GGTODO comment
	const auto& fb = m_layer->get_framebuffer();
	const ImVec2 win_min = ImGui::GetWindowContentRegionMin(), win_max = ImGui::GetWindowContentRegionMax();
	const f32 win_w = win_max.x - win_min.x, win_h = win_max.y - win_min.y;
	const f32 fb_w = fb.get_width<f32>(), fb_h = fb.get_height<f32>();
	const f32 xr = win_w / fb_w, yr = win_h / fb_h;
	const f32 r = std::min(xr, yr);
	const f32 ox = (win_w - fb_w * r) * 0.5f, oy = (win_h - fb_h * r) * 0.5f;
	ImGui::SetCursorPos(ImVec2(ox + win_min.x, oy + win_min.y));
	ImGui::Image(fb.get_imgui_color_id(), ImVec2(fb_w * r, fb_h * r), ImVec2(0, 1), ImVec2(1, 0));


	ImGuizmo::SetOrthographic(false);
	ImGuizmo::SetDrawlist();
	const auto& win_pos = ImGui::GetWindowPos();
	ImGuizmo::SetRect(win_pos.x, win_pos.y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
	pmat<space::CAMERA, space::CLIP> proj = m_layer->get_proj();
	proj.j[1] *= -1;
	proj.k[3] *= -1;
	tmat<space::OBJECT, space::WORLD> obj = m_layer->get_scene()->get_sg_root()->children[2]->mat;
	ImGuizmo::Manipulate(m_layer->get_view().e, proj.e, ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::LOCAL, obj.e);
	if (ImGuizmo::IsUsing())
	{
		printf("A\n");
	}
	if (!ImGuizmo::IsUsing() && ImGui::IsItemClicked(GLFW_MOUSE_BUTTON_LEFT))
	{
		ImGui_ImplGlfw_MouseButtonCallback(m_mgl_context.window, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0);
		m_layer->enable();
	}
}
