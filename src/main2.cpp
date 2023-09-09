#include "pch.h"
#include "ui/shortcut_menus_layer.h"
#include "ui/imgui_layer.h"
#include "ui/preview_layer.h"
#include "ui/preview_window.h"
#include "ui/scene_graph_window.h"
#include "ui/materials_list_window.h"
#include "ui/properties_window.h"
#include "ui/app_ctx.h"
#include "core/scene_material.h"
#include "ui/vertex_editor_window.h"

int main(int argc, char** argv)
{
	printf("%zu | %zu\n", sizeof(mgl::light), alignof(mgl::light));
	printf("%zu | %zu\n", sizeof(light), alignof(light));
	app_ctx a_ctx;

	shortcut_menus_layer sl(&a_ctx);
	a_ctx.mgl_ctx.add_layer(&sl);

	imgui_layer il(&a_ctx);
	a_ctx.mgl_ctx.add_layer(&il);

	preview_layer pl(&a_ctx);
	pl.disable();
	pl.set_disable_callback([&]()
		{
			a_ctx.mgl_ctx.unlock_cursor();
			il.enable();
		});
	pl.set_enable_callback([&]()
		{
			a_ctx.mgl_ctx.lock_cursor();
			il.disable();
		});
	a_ctx.mgl_ctx.add_layer(&pl);

	preview_window preview(&a_ctx);
	preview.set_enable_callback([&]()
		{
			pl.enable();
		});
	il.add_window(&preview);

	materials_list_window ml_window(&a_ctx);
	il.add_window(&ml_window);
	a_ctx.set_mtls_window(&ml_window);

	scene_graph_window sg_window(&a_ctx);
	il.add_window(&sg_window);
	a_ctx.set_sg_window(&sg_window);

	properties_window prop_window(&a_ctx);
	il.add_window(&prop_window);

	vertex_editor_window vert_window(&a_ctx);
	il.add_window(&vert_window);


	while (true)
	{
		if (!a_ctx.mgl_ctx.is_running())
		{
			if (a_ctx.confirm_unsaved_changes())
			{
				break;
			}
			else
			{
				glfwSetWindowShouldClose(a_ctx.mgl_ctx.window, false);
			}
		}

		a_ctx.mgl_ctx.begin_frame();

		char buf[64] = { 0 };
		sprintf_s(buf, "PowerTranzphormR (%u FPS)", (u32)std::round(a_ctx.mgl_ctx.avg_fps));
		a_ctx.mgl_ctx.set_title(buf);

		a_ctx.mgl_ctx.update_layers();
		a_ctx.mgl_ctx.end_frame();
	}

	return 0;
}
