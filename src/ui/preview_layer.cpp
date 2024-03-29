#include "pch.h"
#include "preview_layer.h"
#include "app_ctx.h"

using namespace mgl;
using namespace hats;

preview_layer::preview_layer(app_ctx* const a_ctx) :
	m_app_ctx(a_ctx)
{}



bool preview_layer::on_frame(const f32 dt)
{
	g::texlib->update(m_app_ctx->mgl_ctx.time.now * 1000);

	const direction<space::CAMERA> move_dir(
		get_key(GLFW_KEY_D) - get_key(GLFW_KEY_A),
		get_key(GLFW_KEY_SPACE) - get_key(GLFW_KEY_LEFT_SHIFT),
		get_key(GLFW_KEY_S) - get_key(GLFW_KEY_W));
	const auto& mouse_delta = get_mouse().delta;
	const f32 speed = m_turbo_mode ? 10.f : (get_key(GLFW_KEY_LEFT_ALT) ? .2f : 1.f);
	m_app_ctx->preview_cam.move(dt, move_dir * speed, mouse_delta.x, mouse_delta.y);

	// const tmat<space::OBJECT, space::WORLD> obj;
	const tmat<space::WORLD, space::CAMERA>& view = m_app_ctx->preview_cam.get_view();
	// const tmat<space::OBJECT, space::CAMERA>& mv = view * obj;
	const pmat<space::CAMERA, space::CLIP>& proj = m_app_ctx->preview_cam.get_proj();
	// const mat<space::OBJECT, space::CLIP>& mvp = proj * mv;
	const mat<space::WORLD, space::CLIP> vp = proj * view;
	// HATODO slow
	// const tmat<space::OBJECT, space::WORLD> normal = obj.invert_copy().transpose_copy();

	// draw world into m_fb
	m_app_ctx->preview_fb.bind();
	m_app_ctx->mgl_ctx.clear();
	m_app_ctx->scene.update(m_app_ctx);
	// const scene_ctx_uniforms& uniforms = { mvp, mv, view, proj, obj, normal, m_app_ctx->preview_cam.get_pos() };
	const scene_ctx_uniforms& uniforms = { vp, view, proj, m_app_ctx->preview_cam.get_pos() };
	m_app_ctx->scene.draw(m_app_ctx->mgl_ctx, uniforms);
	m_app_ctx->draw_vertex_editor_icon();
	m_app_ctx->preview_fb.unbind();

	return false;
}
bool preview_layer::on_key(const s32 key, const s32 scancode, const s32 action, const s32 mods)
{
	if (key == GLFW_KEY_ESCAPE)
	{
		disable();
		return true;
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_CAPS_LOCK)
		m_turbo_mode ^= 1;
	return false;
}
