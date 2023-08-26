#pragma once
#include "pch.h"
#include "carve.h"

constexpr static f32 MIN_PARAM_VALUE = .01f, MAX_PARAM_VALUE = 4096.f, DRAG_PARAM_STEP = .01f;
struct generated_mesh_param
{
	bool is_float, is_drag;
	void* value;
	f32 min, max, speed;
};

class generated_mesh
{
public:
	mesh_t* mesh;
	bool dirty;
public:
	generated_mesh(mesh_t* const m) :
		mesh(m),
		dirty(true)
	{}
	MGL_DCM(generated_mesh);
	virtual ~generated_mesh() {}
public:
	virtual std::unordered_map<std::string, generated_mesh_param> get_params() const
	{
		return {};
	}
	virtual void recompute() {}
};

class generated_cuboid : public generated_mesh
{
public:
	generated_cuboid(mesh_t* const m, const cuboid_options& opts) :
		generated_mesh(m),
		m_options(opts)
	{}
public:
	virtual std::unordered_map<std::string, generated_mesh_param> get_params() const
	{
		return {};
	}
private:
	cuboid_options m_options;
};

class generated_ellipsoid : public generated_mesh
{
public:
	generated_ellipsoid(mesh_t* const m, const ellipsoid_options& opts) :
		generated_mesh(m),
		m_options(opts)
	{}
public:
	virtual std::unordered_map<std::string, generated_mesh_param> get_params() const
	{
		return {
			{ "X Steps", { false, false, (void*)&m_options.num_horizontal_steps, 3.f, 64.f, 1.f } },
			{ "Y Steps", { false, false, (void*)&m_options.num_vertical_steps, 3.f, 64.f, 1.f } },
		};
	}
private:
	ellipsoid_options m_options;
};

class generated_cylinder : public generated_mesh
{
public:
	generated_cylinder(mesh_t* const m, const cylinder_options& opts) :
		generated_mesh(m),
		m_options(opts)
	{}
public:
	virtual std::unordered_map<std::string, generated_mesh_param> get_params() const
	{
		return {
			{ "Steps", { false, false, (void*)&m_options.num_steps, 3.f, 64.f, 1.f } },
		};
	}
private:
	cylinder_options m_options;
};

class generated_cone : public generated_mesh
{
public:
	generated_cone(mesh_t* const m, const cone_options& opts) :
		generated_mesh(m),
		m_options(opts)
	{}
public:
	virtual std::unordered_map<std::string, generated_mesh_param> get_params() const
	{
		return {
			{ "Steps", { false, false, (void*)&m_options.num_steps, 3.f, 64.f, 1.f } },
		};
	}
private:
	cone_options m_options;
};

class generated_torus : public generated_mesh
{
public:
	generated_torus(mesh_t* const m, const torus_options& opts) :
		generated_mesh(m),
		m_options(opts)
	{}
public:
	virtual std::unordered_map<std::string, generated_mesh_param> get_params() const
	{
		return {
			{ "Center Radius", { true, true, (void*)&m_options.center_radius, MIN_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
			{ "Tube Radius", { true, true, (void*)&m_options.tube_radius, MIN_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
			{ "Center Steps", { false, false, (void*)&m_options.num_center_steps, 3.f, 64.f, 1.f } },
			{ "Tube Steps", { false, false, (void*)&m_options.num_tube_steps, 3.f, 64.f, 1.f } },
		};
	}
private:
	torus_options m_options;
};

class generated_heightmap : public generated_mesh
{
public:
	generated_heightmap(mesh_t* const m, const heightmap_options& opts) :
		generated_mesh(m),
		m_options(opts)
	{}
	virtual std::unordered_map<std::string, generated_mesh_param> get_params() const
	{
		// TODO
		assert(false);
		return {};
	}
private:
	heightmap_options m_options;
};
