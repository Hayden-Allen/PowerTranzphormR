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
	const mgl::camera& get_camera() const
	{
		return m_cam;
	}
	scene_ctx* get_scene()
	{
		return m_scene;
	}
	void set_selected_node(sgnode* const node)
	{
		m_selected = node;
	}
	sgnode* get_selected_node()
	{
		return m_selected;
	}
private:
	const mgl::context* m_mgl_context;
	scene_ctx* m_scene;
	mgl::framebuffer_u8 m_fb;
	mgl::camera m_cam;
	action_stack m_actions;
	sgnode* m_selected;
};
