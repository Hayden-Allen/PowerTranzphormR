#include "pch.h"
#include "preview_layer.h"

using namespace mgl;
using namespace hats;

preview_layer::preview_layer(const mgl::context* const mgl_context, scene_ctx* scene) :
	m_mgl_context(mgl_context),
	m_scene(scene),
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
	const f32 tdx = 1.f * get_key(GLFW_KEY_RIGHT) - get_key(GLFW_KEY_LEFT);
	const f32 tdy = 1.f * get_key(GLFW_KEY_UP) - get_key(GLFW_KEY_DOWN);
	if (tdx != 0 || tdy != 0)
	{
		// TODO horrible
		m_scene->get_sg_root()->children[2]->transform(m_scene->get_csg(), carve::math::Matrix::TRANS(tdx * dt, tdy * dt, 0));
	}

	const direction<space::CAMERA> move_dir(
		get_key(GLFW_KEY_D) - get_key(GLFW_KEY_A),
		get_key(GLFW_KEY_SPACE) - get_key(GLFW_KEY_LEFT_CONTROL),
		get_key(GLFW_KEY_S) - get_key(GLFW_KEY_W));
	const auto& mouse_delta = get_mouse().delta;
	m_cam.move(dt, move_dir * (get_key(GLFW_KEY_LEFT_SHIFT) ? .2f : 1.f), mouse_delta.x, mouse_delta.y);

	const tmat<space::OBJECT, space::WORLD> obj;
	const mat<space::OBJECT, space::CLIP>& mvp = m_cam.get_view_proj() * obj;

	m_fb.bind();
	glEnable(GL_DEPTH_TEST);
	m_mgl_context->clear();
	m_scene->update();
	m_scene->draw(*m_mgl_context, mvp);
	m_fb.unbind();
}

void preview_layer::on_key(const s32 key, const s32 scancode, const s32 action, const s32 mods)
{
	if (key == GLFW_KEY_ESCAPE)
		disable();
}
