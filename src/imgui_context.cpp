#include "pch.h"
#include "imgui_context.h"

imgui_context::imgui_context(const mgl::context& mgl_context) : mgl::layer(), m_mgl_context(mgl_context) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
	ImGui_ImplGlfw_InitForOpenGL(m_mgl_context.window, false);
	ImGui_ImplOpenGL3_Init("#version 430 core");
}

imgui_context::~imgui_context() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void imgui_context::on_frame(const f32 dt)
{
	ImGui::GetIO().DeltaTime = dt;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	if (ImGui::Begin("imgui_context_dockspace", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus))
	{
		if (m_menus.size() > 0) {
			if (ImGui::BeginMenuBar()) {
				for (const imgui_menu& menu : m_menus) {
					if (ImGui::BeginMenu(menu.name.c_str())) {
						for (size_t i = 0; i < menu.groups.size(); ++i) {
							const imgui_menu_item_group& group = menu.groups[i];
							for (const imgui_menu_item& item : group) {
								if (ImGui::MenuItem(item.name.c_str(), item.shortcut_text.c_str())) {
									item.handler();
								}
							}

							if (i != menu.groups.size() - 1) {
								ImGui::Separator();
							}
						}

						ImGui::EndMenu();
					}
				}
				ImGui::EndMenuBar();
			}
		}

		ImGuiID dockspace_id = ImGui::GetID("imgui_context_dockspace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.f, 0.f), ImGuiDockNodeFlags_None);

		for (imgui_window* window : m_windows) {
			if (ImGui::Begin(window->title.c_str())) {
				window->handle_frame();
				ImGui::End();
			}
		}

		ImGui::End();
	}
	ImGui::PopStyleVar(2);

	ImGui::Render();

	glDisable(GL_DEPTH_TEST);
	m_mgl_context.clear();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}
}

void imgui_context::add_window(imgui_window* window)
{
	remove_window(window);
	m_windows.push_back(window);
}

void imgui_context::remove_window(imgui_window* window)
{
	for (size_t i = 0; i < m_windows.size(); ++i) {
		if (m_windows[i] == window) {
			m_windows.erase(m_windows.begin() + i);
			break;
		}
	}
}

void imgui_context::set_menus(const std::vector<imgui_menu>& menus) {
	m_menus = menus;
}

void imgui_context::on_window_resize(const s32 width, const s32 height)
{
	ImGui::GetIO().DisplaySize = ImVec2(MGL_CAST(f32, width), MGL_CAST(f32, height));
}

void imgui_context::on_mouse_button(const s32 button, const s32 action, const s32 mods)
{
	ImGuiIO& io = ImGui::GetIO();
	if (button < ImGuiMouseButton_COUNT) {
		if (action == GLFW_PRESS) {
			io.MouseDown[button] = true;
		}
		else if (action == GLFW_RELEASE) {
			io.MouseDown[button] = false;
		}
	}
}

void imgui_context::on_mouse_move(const f32 x, const f32 y, const f32 dx, const f32 dy)
{
	//
	// FIXME
	//
	// printf("%f %f | %f %f\n", x, y, dx, dy);
	ImGui::GetIO().MousePos = ImVec2(x, y);
}

void imgui_context::on_scroll(const f32 x, const f32 y)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MouseWheel += y;
	io.MouseWheelH += x;
}

void imgui_context::on_key(const s32 key, const s32 scancode, const s32 action, const s32 mods)
{
	if ((action != GLFW_PRESS && action != GLFW_RELEASE) || key < 0 || key >= ImGuiKey_COUNT) {
		return;
	}

	ImGuiIO& io = ImGui::GetIO();
	bool pressed = action == GLFW_PRESS;

	io.KeysDown[key] = pressed;
	io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
	io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
	io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
	io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
}
