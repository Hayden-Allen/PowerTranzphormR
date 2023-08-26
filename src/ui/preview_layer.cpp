#include "pch.h"
#include "preview_layer.h"

using namespace mgl;
using namespace hats;

preview_layer::preview_layer(app_ctx* const a_ctx) :
	m_app_ctx(a_ctx)
{
}

preview_layer::~preview_layer()
{
}

bool preview_layer::on_frame(const f32 dt)
{
	const direction<space::CAMERA> move_dir(
		get_key(GLFW_KEY_D) - get_key(GLFW_KEY_A),
		get_key(GLFW_KEY_SPACE) - get_key(GLFW_KEY_LEFT_SHIFT),
		get_key(GLFW_KEY_S) - get_key(GLFW_KEY_W));
	const auto& mouse_delta = get_mouse().delta;
	m_app_ctx->preview_cam.move(dt, move_dir * (get_key(GLFW_KEY_LEFT_ALT) ? .2f : 1.f), mouse_delta.x, mouse_delta.y);

	const tmat<space::OBJECT, space::WORLD> obj;
	const tmat<space::OBJECT, space::CAMERA>& mv = m_app_ctx->preview_cam.get_view() * obj;
	const mat<space::OBJECT, space::CLIP>& mvp = m_app_ctx->preview_cam.get_proj() * mv;
	// HATODO slow
	const tmat<space::OBJECT, space::WORLD> normal = obj.invert_copy().transpose_copy();

	// draw world into m_fb
	m_app_ctx->preview_fb.bind();
	m_app_ctx->mgl_ctx.clear();
	m_app_ctx->scene.update();
	const scene_ctx_uniforms& uniforms = { mvp, mv, obj, normal, m_app_ctx->preview_cam.get_pos() };
	m_app_ctx->scene.draw(m_app_ctx->mgl_ctx, uniforms);
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
	return false;
}
