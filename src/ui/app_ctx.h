#pragma once

#include <mingl/mingl.h>
#include "scene_ctx.h"
#include "action_stack.h"

struct app_ctx {
	scene_ctx scene;
	action_stack actions;
	mgl::context mgl_ctx;
	mgl::framebuffer_u8 preview_fb;
	mgl::camera preview_cam;
	app_ctx() :
		actions(&scene),
		mgl_ctx(1280, 720, "PowerTranzphormR", { .vsync = true, .clear = { .b = 1.f } }),
		preview_fb(1280, 720)
	{
		f32 ar = static_cast<f32>(preview_fb.get_width()) / static_cast<f32>(preview_fb.get_height());
		preview_cam = mgl::camera({ 0, 0, 5 }, 0, 0, 108 / ar, ar, 0.1f, 1000.0f, 5.0f);
	}
	void transform_action(sgnode* const t, const tmat<space::OBJECT, space::WORLD>& old_mat, const tmat<space::OBJECT, space::WORLD>& new_mat)
	{
		actions.transform(t, old_mat, new_mat);
	}
	void reparent_action(sgnode* const target, sgnode* const old_parent, sgnode* const new_parent)
	{
		actions.reparent(target, old_parent, new_parent);
	}
	void create_action(sgnode* const target, sgnode* const parent)
	{
		actions.create(target, parent);
	}
	void destroy_action(sgnode* const target)
	{
		actions.destroy(target);
	}
	void undo()
	{
		actions.undo();
	}
	void redo()
	{
		actions.redo();
	}
};
