#include "pch.h"
#include "imgui_layer.h"
#include "preview_layer.h"
#include "preview_window.h"

const std::vector<imgui_menu> construct_app_menus()
{
	std::vector<imgui_menu> result;

	imgui_menu file_menu;
	file_menu.name = "File";
	imgui_menu_item file_new = {
		"New Phonky Phorm",
		[]()
		{
			std::cout << "MENU: NEW\n";
			},
		"Ctrl+N",
		{ GLFW_KEY_LEFT_CONTROL, GLFW_KEY_N }
	};
	imgui_menu_item file_open = {
		"Open Phonky Phorm",
		[]()
		{
			std::cout << "MENU: OPEN\n";
			},
		"Ctrl+O",
		{ GLFW_KEY_LEFT_CONTROL, GLFW_KEY_O }
	};
	imgui_menu_item file_save = {
		"Save Phonky Phorm",
		[]()
		{
			std::cout << "MENU: SAVE\n";
			},
		"Ctrl+S",
		{ GLFW_KEY_LEFT_CONTROL, GLFW_KEY_S }
	};
	imgui_menu_item file_save_as = {
		"Save Phonky Phorm As...",
		[]()
		{
			std::cout << "MENU: SAVE AS\n";
			},
		"Ctrl+Shift+S",
		{ GLFW_KEY_LEFT_CONTROL, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_S }
	};
	file_menu.groups.push_back({ file_new, file_open, file_save, file_save_as });
	result.push_back(file_menu);

	imgui_menu edit_menu;
	edit_menu.name = "Edit";
	imgui_menu_item edit_undo = {
		"Undo Tranzphormation",
		[]()
		{
			std::cout << "MENU: UNDO\n";
			},
		"Ctrl+Z",
		{ GLFW_KEY_LEFT_CONTROL, GLFW_KEY_Z }
	};
	imgui_menu_item edit_redo = {
		"Redo Tranzphormation",
		[]()
		{
			std::cout << "MENU: REDO\n";
			},
		"Ctrl+Y",
		{ GLFW_KEY_LEFT_CONTROL, GLFW_KEY_Y }
	};
	edit_menu.groups.push_back({ edit_undo, edit_redo });
	result.push_back(edit_menu);

	return result;
}

int main(int argc, char** argv)
{
	LOG_LEVEL(warn);
	mgl::context c(1280, 720, "PowerTranzphormR", true);
	c.set_clear_color(0, 0, 1);
	imgui_layer il(c);
	il.set_menus(construct_app_menus());
	c.add_layer(&il);
	preview_layer pl(c, 1280, 720);
	pl.set_enabled(false);
	pl.set_exit_preview_callback([&]()
								 {
									 c.set_pointer_locked(false);
									 pl.set_enabled(false);
									 il.set_enabled(true);
								 });
	c.add_layer(&pl);
	preview_window preview(c, pl.get_framebuffer());
	preview.set_enter_preview_callback([&]()
									   {
										   c.set_pointer_locked(true);
										   il.set_enabled(false);
										   pl.set_enabled(true);
									   });
	il.add_window(&preview);

	constexpr u32 NUM_FPS_SAMPLES = 128;
	f32 fps_samples[NUM_FPS_SAMPLES] = { 0.f };
	s32 cur_sample = 0;
	f32 avg_fps = 0.f;

	while (c.is_running())
	{
		c.begin_frame();
		const f32 cur_fps = 1.f / c.time.delta;
		avg_fps -= fps_samples[cur_sample] / NUM_FPS_SAMPLES;
		fps_samples[cur_sample] = cur_fps;
		avg_fps += fps_samples[cur_sample] / NUM_FPS_SAMPLES;
		cur_sample = (cur_sample + 1) % NUM_FPS_SAMPLES;
		std::string window_title = "PowerTranzphormR (" +
								   std::to_string((u32)std::round(avg_fps)) +
								   " FPS)";
		glfwSetWindowTitle(c.window, window_title.c_str());
		c.handle_layers_for_frame();
		c.end_frame();
	}

	return 0;
}
