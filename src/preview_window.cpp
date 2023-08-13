#include "preview_window.h"
template<typename FB>
preview_window<FB>::preview_window(const mgl::context &mgl_context, const FB& framebuffer) :
	m_mgl_context(mgl_context),
	m_fb(framebuffer)
{
	title = "Preview";
}

template<typename FB>
preview_window<FB>::~preview_window()
{}

template<typename FB>
void preview_window<FB>::set_enter_preview_callback(const std::function<void()>& callback)
{
	m_enter_preview_callback = callback;
}

template<typename FB>
void preview_window<FB>::handle_frame()
{
	ImVec2 win_min = ImGui::GetWindowContentRegionMin(), win_max = ImGui::GetWindowContentRegionMax();
	f32 win_w = win_max.x - win_min.x, win_h = win_max.y - win_min.y;
	f32 fb_w = m_fb.get_width<f32>(), fb_h = m_fb.get_height<f32>();
	f32 xr = win_w / fb_w, yr = win_h / fb_h;
	f32 r = std::min(xr, yr);
	f32 ox = (win_w - fb_w * r) * 0.5f, oy = (win_h - fb_h * r) * 0.5f;
	ImGui::SetCursorPos(ImVec2(ox + win_min.x, oy + win_min.y));
	ImGui::Image(m_fb.get_imgui_color_id(), ImVec2(fb_w * r, fb_h * r), ImVec2(0, 1), ImVec2(1, 0));
	if (ImGui::IsItemClicked(GLFW_MOUSE_BUTTON_LEFT))
	{
		ImGui_ImplGlfw_MouseButtonCallback(m_mgl_context.window, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0);
		m_enter_preview_callback();
	}
}
