#include "pch.h"
#include "preview_layer.h"

using namespace mgl;
using namespace hats;

preview_layer::preview_layer(const mgl::context* const mgl_context, scene_ctx* const scene, imgui_layer* const il) :
	m_mgl_context(mgl_context),
	m_scene(scene),
	m_imgui_layer(il),
	m_fb(mgl_context->get_width(), mgl_context->get_height())
{
	f32 ar = (f32)m_fb.get_width() / (f32)m_fb.get_height();
	m_cam = camera({ 0, 0, 5 }, 0, 0, 108 / ar, ar, 0.1f, 1000.0f, 5.0f);
}

preview_layer::~preview_layer()
{
}

void preview_layer::on_frame(const f32 dt)
{
	const direction<space::CAMERA> move_dir(
		get_key(GLFW_KEY_D) - get_key(GLFW_KEY_A),
		get_key(GLFW_KEY_SPACE) - get_key(GLFW_KEY_LEFT_SHIFT),
		get_key(GLFW_KEY_S) - get_key(GLFW_KEY_W));
	const auto& mouse_delta = get_mouse().delta;
	m_cam.move(dt, move_dir * (get_key(GLFW_KEY_LEFT_ALT) ? .2f : 1.f), mouse_delta.x, mouse_delta.y);

	const tmat<space::OBJECT, space::WORLD> obj;
	const tmat<space::OBJECT, space::CAMERA>& mv = m_cam.get_view() * obj;
	const mat<space::OBJECT, space::CLIP>& mvp = m_cam.get_proj() * mv;
	// HATODO slow
	const tmat<space::OBJECT, space::WORLD> normal = obj.invert_copy().transpose_copy();

	// draw world into m_fb
	m_fb.bind();
	m_mgl_context->clear();
	m_scene->update();
	const scene_ctx_uniforms& uniforms = { mvp, mv, obj, normal, m_cam.get_pos() };
	m_scene->draw(*m_mgl_context, uniforms);
	m_fb.unbind();
}

void preview_layer::on_key(const s32 key, const s32 scancode, const s32 action, const s32 mods)
{
	if (key == GLFW_KEY_ESCAPE)
		disable();

	if (mods & GLFW_MOD_CONTROL)
	{
		if (key == GLFW_KEY_Z)
			m_imgui_layer->undo();
		if (key == GLFW_KEY_Y)
			m_imgui_layer->redo();
	}
}
