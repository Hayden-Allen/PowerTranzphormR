#include "pch.h"
#include "preview_window.h"

preview_window::preview_window(const mgl::framebuffer_u8& framebuffer) : imgui_window(), m_fb(framebuffer) {
	title = "Preview";
}

preview_window::~preview_window() {
	//
}

void preview_window::handle_frame() {
	ImVec2 win_min = ImGui::GetWindowContentRegionMin(), win_max = ImGui::GetWindowContentRegionMax();
	f32 win_w = win_max.x - win_min.x, win_h = win_max.y - win_min.y;
	f32 fb_w = m_fb.get_width(), fb_h = m_fb.get_height();
	f32 xr = win_w / fb_w, yr = win_h / fb_h;
	f32 r = std::min(xr, yr);
	f32 ox = (win_w - fb_w * r) * 0.5f, oy = (win_h - fb_h * r) * 0.5f;
	ImGui::SetCursorPos(ImVec2(ox + win_min.x, oy + win_min.y));
	ImGui::Image(m_fb.get_imgui_color_id(), ImVec2(fb_w * r, fb_h * r), ImVec2(0, 1), ImVec2(1, 0));
}
