#include "pch.h"
#include "imgui_layer.h"
#include "preview_layer.h"
#include "preview_window.h"

int main(int argc, char** argv)
{
	mgl::context c(1280, 720, "PowerTranzphormR",
		{ .vsync = true,
			.clear = { .b = 1.f } });

	imgui_layer il(&c);
	c.add_layer(&il);

	const f32 ar = c.get_aspect_ratio();
	camera cam({ 0, 0, 5 }, 0, 0, 108 / ar, ar, 0.1f, 1000.0f, 5.0f);
	preview_layer pl(&c, cam);
	pl.disable();
	pl.set_disable_callback([&]()
		{
			c.unlock_cursor();
			il.enable();
		});
	pl.set_enable_callback([&]()
		{
			c.lock_cursor();
			il.disable();
		});
	c.add_layer(&pl);

	preview_window preview(c, &pl);
	il.add_window(&preview);

	while (c.is_running())
	{
		c.begin_frame();

		char buf[64] = { 0 };
		sprintf_s(buf, "PowerTranzphormR (%u FPS)", (u32)std::round(c.avg_fps));
		c.set_title(buf);

		c.update_layers();
		c.end_frame();
	}

	return 0;
}
