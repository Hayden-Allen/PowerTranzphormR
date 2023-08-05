#pragma once
#include "pch.h"

using namespace hats;

class camera
{
public:
	camera(const point<space::WORLD>& pos, const direction<space::WORLD>& dir, const f32 fov_y, const f32 aspect, const f32 near, const f32 far, const f32 speed, const direction<space::WORLD>& up = direction_util::j_hat<space::WORLD>()) :
		m_pos(pos),
		m_dir(dir),
		m_up(up),
		m_fov_y(fov_y),
		m_aspect(aspect),
		m_near(near),
		m_far(far),
		m_speed(speed),
		m_proj(pmat_util::projection(fov_y, aspect, near, far)),
		m_view(tmat_util::look_at<space::WORLD, space::CAMERA>(m_pos, m_dir, m_up)),
		m_vp(m_proj * m_view)
	{}
public:
	void move(const f32 dt, const vec<space::WORLD>& amount)
	{
		m_pos += m_speed * dt * amount;
		m_view = tmat_util::look_at<space::WORLD, space::CAMERA>(m_pos, m_dir, m_up);
		m_vp = m_proj * m_view;
	}
	pmat<space::CAMERA, space::CLIP> get_proj() const
	{
		return m_proj;
	}
	tmat<space::WORLD, space::CAMERA> get_view() const
	{
		return m_view;
	}
	mat<space::WORLD, space::CLIP> get_view_proj() const
	{
		return m_vp;
	}
private:
	point<space::WORLD> m_pos;
	direction<space::WORLD> m_dir;
	direction<space::WORLD> m_up;
	f32 m_fov_y, m_aspect, m_near, m_far, m_speed;
	pmat<space::CAMERA, space::CLIP> m_proj;
	tmat<space::WORLD, space::CAMERA> m_view;
	mat<space::WORLD, space::CLIP> m_vp;
};