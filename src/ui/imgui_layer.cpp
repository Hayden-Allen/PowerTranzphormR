#include "pch.h"
#include "imgui_layer.h"
#include "app_ctx.h"

imgui_layer::imgui_layer(app_ctx* const a_ctx) :
	m_app_ctx(a_ctx)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
	ImGui_ImplGlfw_InitForOpenGL(m_app_ctx->mgl_ctx.window, false);
	ImGui_ImplOpenGL3_Init("#version 430 core");

	// See also: https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/ImGui/ImGuiLayer.cpp
	auto& colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };
	colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
	colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
	colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
	colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
	colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_CheckMark] = ImVec4{ 0.48f, 0.4805f, 0.481f, 1.0f };
	colors[ImGuiCol_SliderGrab] = ImVec4{ 0.48f, 0.4805f, 0.481f, 1.0f };
	colors[ImGuiCol_SliderGrabActive] = ImVec4{ 0.48f, 0.4805f, 0.481f, 1.0f };
	colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
	colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.25f, 0.255f, 0.26f, 1.0f };
	colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.25f, 0.255f, 0.26f, 1.0f };
	colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
	colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
	colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
	colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_MenuBarBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
}
imgui_layer::~imgui_layer()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}



bool imgui_layer::on_frame(const f32 dt)
{
	f32 x_scale = -1.0f, y_scale = -1.0f;
	glfwGetWindowContentScale(m_app_ctx->mgl_ctx.window, &x_scale, &y_scale);
	if (x_scale != m_prev_x_scale || y_scale != m_prev_y_scale)
	{
		ImGui_ImplOpenGL3_DestroyFontsTexture();
		ImGui::GetIO().Fonts->Clear();
		g::font_size = 20.0f * std::max(x_scale, y_scale);
		ImGui::GetIO().Fonts->AddFontFromFileTTF("res/fonts/Jost-Regular.ttf", g::font_size);
		ImGui::GetIO().Fonts->Build();
		ImGui_ImplOpenGL3_CreateFontsTexture();
		m_prev_x_scale = x_scale;
		m_prev_y_scale = y_scale;
	}

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	if (ImGui::Begin("imgui_layer_dockspace", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus))
	{
		ImGui::PopStyleVar(1);

		draw_menus();

		ImGuiID dockspace_id = ImGui::GetID("imgui_layer_dockspace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.f, 0.f), ImGuiDockNodeFlags_None);

		imgui_window* focused_window = nullptr;
		for (imgui_window* window : m_windows)
		{
			bool should_draw_window = ImGui::Begin(window->get_title().c_str());
			if (ImGui::IsWindowFocused())
			{
				focused_window = window;
			}
			if (should_draw_window)
			{
				window->handle_frame();
			}
			ImGui::End();
		}
		if (focused_window && m_prev_focused_window != focused_window)
		{
			if (m_prev_focused_window)
			{
				m_prev_focused_window->handle_focused(false);
			}
			focused_window->handle_focused(true);
			m_prev_focused_window = focused_window;
		}
	}
	ImGui::End();
	ImGui::PopStyleVar(2);

	ImGui::Render();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	m_app_ctx->mgl_ctx.clear();
	m_app_ctx->mgl_ctx.reset_viewport();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	return false;
}
bool imgui_layer::on_mouse_button(const s32 button, const s32 action, const s32 mods)
{
	ImGui_ImplGlfw_MouseButtonCallback(m_app_ctx->mgl_ctx.window, button, action, mods);
	return false;
}
bool imgui_layer::on_mouse_move(const f32 x, const f32 y, const f32 dx, const f32 dy)
{
	ImGui_ImplGlfw_CursorPosCallback(m_app_ctx->mgl_ctx.window, x, y);
	return false;
}
bool imgui_layer::on_scroll(const f32 x, const f32 y)
{
	ImGui_ImplGlfw_ScrollCallback(m_app_ctx->mgl_ctx.window, x, y);
	return false;
}
bool imgui_layer::on_key(const s32 key, const s32 scancode, const s32 action, const s32 mods)
{
	ImGui_ImplGlfw_KeyCallback(m_app_ctx->mgl_ctx.window, key, scancode, action, mods);
	return false;
}
bool imgui_layer::on_char(const u32 codepoint)
{
	ImGui_ImplGlfw_CharCallback(m_app_ctx->mgl_ctx.window, codepoint);
	return false;
}
void imgui_layer::add_window(imgui_window* window)
{
	remove_window(window);
	m_windows.push_back(window);
}
void imgui_layer::remove_window(imgui_window* window)
{
	for (size_t i = 0; i < m_windows.size(); ++i)
	{
		if (m_windows[i] == window)
		{
			m_windows.erase(m_windows.begin() + i);
			break;
		}
	}
}



void imgui_layer::draw_menus()
{
	if (m_app_ctx->shortcut_menus.size() > 0 && ImGui::BeginMenuBar())
	{
		for (const shortcut_menu& menu : m_app_ctx->shortcut_menus)
		{
			draw_menu(menu);
		}
		ImGui::EndMenuBar();
	}
}
void imgui_layer::draw_menu(const shortcut_menu& menu)
{
	if (ImGui::BeginMenu(menu.name.c_str()))
	{
		for (size_t i = 0; i < menu.groups.size(); ++i)
		{
			const shortcut_menu_item_group& group = menu.groups[i];
			for (const shortcut_menu_item& item : group)
			{
				draw_menu_item(item);
			}

			if (i != menu.groups.size() - 1)
			{
				ImGui::Separator();
			}
		}
		ImGui::EndMenu();
	}
}
void imgui_layer::draw_menu_item(const shortcut_menu_item& item)
{
	if (item.groups.size())
	{
		const std::string name = item.groups.size() ? (item.name + std::string("...")) : item.name;
		if (ImGui::BeginMenu(name.c_str(), item.enabled()))
		{
			for (size_t i = 0; i < item.groups.size(); ++i)
			{
				const shortcut_menu_item_group& group = item.groups[i];
				for (const shortcut_menu_item& item : group)
				{
					draw_menu_item(item);
				}
				if (i != item.groups.size() - 1)
				{
					ImGui::Separator();
				}
			}
			ImGui::EndMenu();
		}
	}
	else if (ImGui::MenuItem(item.name.c_str(), item.keys_text.c_str(), nullptr, item.enabled()))
	{
		item.handler();
	}
}
