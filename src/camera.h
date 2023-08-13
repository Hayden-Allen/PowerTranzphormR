#pragma once
#include "pch.h"

using namespace hats;

class camera
{
public:
	camera(const point<space::WORLD>& pos, const f32 angle_x, const f32 angle_y, const f32 fov_y, const f32 aspect, const f32 near, const f32 far, const f32 speed, const direction<space::WORLD>& up = direction_util::j_hat<space::WORLD>()) :
		m_pos(pos),
		m_up(up),
		/*m_fov_y(fov_y),
		m_aspect(aspect),
		m_near(near),
		m_far(far),*/
		m_angle_x(angle_x),
		m_angle_y(angle_y),
		m_speed(speed),
		m_proj(pmat_util::projection(fov_y, aspect, near, far)),
		m_view(tmat_util::translation<space::WORLD, space::CAMERA>(m_pos)),
		m_vp(m_proj * m_view)
	{}
public:
	void move(const f32 dt, vec<space::CAMERA> amount, const f32 mx, const f32 my)
	{
		const f32 ls = std::clamp(1.f * dt, -.005f, .005f);
		m_angle_x += ls * my;
		m_angle_y += ls * mx;
		if (fabs(m_angle_x) >= c::PI / 2)
		{
			m_angle_x -= ls * my;
		}
		m_angle_y = clean_angle(m_angle_y);

		// make lateral movement relative to camera's y-axis angle
		// this means that wasd always move in the xz-plane and space/shift always move on the y-axis
		vec<space::WORLD> wamount = amount.transform_copy(tmat_util::rotation_y<space::CAMERA, space::WORLD>(-m_angle_y));
		m_pos += wamount * m_speed * dt;
		m_view = tmat_util::rotation_x<space::CAMERA>(m_angle_x) *
				 tmat_util::rotation_y<space::CAMERA>(m_angle_y) *
				 tmat_util::translation<space::WORLD, space::CAMERA>(m_pos);
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
	const point<space::WORLD>& get_pos() const
	{
		return m_pos;
	}
private:
	point<space::WORLD> m_pos;
	direction<space::WORLD> m_up;
	// f32 m_fov_y, m_aspect, m_near, m_far, m_speed;
	f32 m_angle_x, m_angle_y, m_speed;
	pmat<space::CAMERA, space::CLIP> m_proj;
	tmat<space::WORLD, space::CAMERA> m_view;
	mat<space::WORLD, space::CLIP> m_vp;
};