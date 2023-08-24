#pragma once
#include "scene_ctx.h"
#include "action_stack.h"

class preview_layer : public mgl::layer
{
public:
	preview_layer(const mgl::context* const mgl_context, scene_ctx* scene);
	virtual ~preview_layer();
public:
	virtual void on_frame(const f32 dt) override;
	virtual void on_key(const s32 key, const s32 scancode, const s32 action, const s32 mods) override;
	const mgl::framebuffer_u8& get_framebuffer() const
	{
		return m_fb;
	}
	const tmat<space::WORLD, space::CAMERA>& get_view() const
	{
		return m_cam.get_view();
	}
	const pmat<space::CAMERA, space::CLIP>& get_proj() const
	{
		return m_cam.get_proj();
	}
	scene_ctx* get_scene()
	{
		return m_scene;
	}
private:
	const mgl::context* m_mgl_context;
	scene_ctx* m_scene;
	mgl::framebuffer_u8 m_fb;
	mgl::camera m_cam;
	action_stack m_actions;
};
