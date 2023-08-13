#include "pch.h"
#include "imgui_layer.h"

imgui_layer::imgui_layer(const mgl::context& mgl_context) :
	mgl::layer(), m_mgl_context(mgl_context)
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
	io.Fonts->AddFontFromFileTTF("res/fonts/Jost-Regular.ttf", 20);
	ImGui_ImplGlfw_InitForOpenGL(m_mgl_context.window, false);
	ImGui_ImplOpenGL3_Init("#version 430 core");

	// https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/ImGui/ImGuiLayer.cpp
	auto& colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };
	colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
	colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
	colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
	colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
	colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
	colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
	colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
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

void imgui_layer::on_frame(const f32 dt)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

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

		for (imgui_window* window : m_windows)
		{
			if (ImGui::Begin(window->title.c_str()))
			{
				window->handle_frame();
			}
			ImGui::End();
		}
	}
	ImGui::End();
	ImGui::PopStyleVar(2);

	ImGui::Render();

	glDisable(GL_DEPTH_TEST);
	m_mgl_context.clear();
	glViewport(0, 0, m_mgl_context.width, m_mgl_context.height);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}
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

void imgui_layer::set_menus(const std::vector<imgui_menu>& menus)
{
	m_menus = menus;
}

void imgui_layer::on_window_resize(const s32 width, const s32 height)
{
	//
}

void imgui_layer::on_mouse_button(const s32 button, const s32 action, const s32 mods)
{
	ImGui_ImplGlfw_MouseButtonCallback(m_mgl_context.window, button, action, mods);
}

void imgui_layer::on_mouse_move(const f32 x, const f32 y, const f32 dx, const f32 dy)
{
	ImGui_ImplGlfw_CursorPosCallback(m_mgl_context.window, x, y);
}

void imgui_layer::on_scroll(const f32 x, const f32 y)
{
	ImGui_ImplGlfw_ScrollCallback(m_mgl_context.window, x, y);
}

void imgui_layer::on_key(const s32 key, const s32 scancode, const s32 action, const s32 mods)
{
	ImGui_ImplGlfw_KeyCallback(m_mgl_context.window, key, scancode, action, mods);
}

void imgui_layer::draw_menus()
{
	if (m_menus.size() > 0 && ImGui::BeginMenuBar())
	{
		for (const imgui_menu& menu : m_menus)
		{
			draw_menu(menu);
		}
		ImGui::EndMenuBar();
	}
}

void imgui_layer::draw_menu(const imgui_menu& menu)
{
	if (ImGui::BeginMenu(menu.name.c_str()))
	{
		for (size_t i = 0; i < menu.groups.size(); ++i)
		{
			const imgui_menu_item_group& group = menu.groups[i];
			for (const imgui_menu_item& item : group)
			{
				if (ImGui::MenuItem(item.name.c_str(), item.shortcut_text.c_str()))
				{
					item.handler();
				}
			}

			if (i != menu.groups.size() - 1)
			{
				ImGui::Separator();
			}
		}
		ImGui::EndMenu();
	}
}