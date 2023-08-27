#pragma once
#include "pch.h"
#include "carve.h"
#include "scene_ctx.h"

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
	virtual void recompute(scene_ctx* const scene) {}
	virtual generated_mesh* clone() const
	{
		return new generated_mesh(nullptr);
	}
};

class generated_cuboid : public generated_mesh
{
public:
	generated_cuboid(scene_ctx* const scene, const GLuint material, const cuboid_options& opts) :
		generated_mesh(nullptr),
		m_material(material),
		m_options(opts)
	{
		recompute(scene);
	}
public:
	std::unordered_map<std::string, generated_mesh_param> get_params() const override
	{
		return {};
	}
	void recompute(scene_ctx* const scene) override
	{
		// delete mesh;
		mesh = scene->create_textured_cuboid(m_material, m_options);
	}
	virtual generated_mesh* clone() const
	{
		return new generated_cuboid(m_material, m_options);
	}
private:
	generated_cuboid(const GLuint material, const cuboid_options& opts) :
		generated_mesh(nullptr),
		m_material(material),
		m_options(opts)
	{}
private:
	GLuint m_material;
	cuboid_options m_options;
};

class generated_ellipsoid : public generated_mesh
{
public:
	generated_ellipsoid(scene_ctx* const scene, const GLuint material, const ellipsoid_options& opts) :
		generated_mesh(nullptr),
		m_material(material),
		m_options(opts)
	{
		recompute(scene);
	}
public:
	std::unordered_map<std::string, generated_mesh_param> get_params() const override
	{
		return {
			{ "X Steps", { false, false, (void*)&m_options.num_horizontal_steps, 3.f, 64.f, 1.f } },
			{ "Y Steps", { false, false, (void*)&m_options.num_vertical_steps, 3.f, 64.f, 1.f } },
		};
	}
	void recompute(scene_ctx* const scene) override
	{
		// delete mesh;
		mesh = scene->create_textured_ellipsoid(m_material, m_options);
	}
	virtual generated_mesh* clone() const
	{
		return new generated_ellipsoid(m_material, m_options);
	}
private:
	generated_ellipsoid(const GLuint material, const ellipsoid_options& opts) :
		generated_mesh(nullptr),
		m_material(material),
		m_options(opts)
	{}
private:
	GLuint m_material;
	ellipsoid_options m_options;
};

class generated_cylinder : public generated_mesh
{
public:
	generated_cylinder(scene_ctx* const scene, const GLuint material, const cylinder_options& opts) :
		generated_mesh(nullptr),
		m_material(material),
		m_options(opts)
	{
		recompute(scene);
	}
public:
	std::unordered_map<std::string, generated_mesh_param> get_params() const override
	{
		return {
			{ "Steps", { false, false, (void*)&m_options.num_steps, 3.f, 64.f, 1.f } },
			{ "Top Radius", { true, true, (void*)&m_options.top_radius, MIN_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
			{ "Bottom Radius", { true, true, (void*)&m_options.bottom_radius, MIN_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
		};
	}
	void recompute(scene_ctx* const scene) override
	{
		// delete mesh;
		mesh = scene->create_textured_cylinder(m_material, m_options);
	}
	virtual generated_mesh* clone() const
	{
		return new generated_cylinder(m_material, m_options);
	}
private:
	generated_cylinder(const GLuint material, const cylinder_options& opts) :
		generated_mesh(nullptr),
		m_material(material),
		m_options(opts)
	{}
private:
	GLuint m_material;
	cylinder_options m_options;
};

class generated_cone : public generated_mesh
{
public:
	generated_cone(scene_ctx* const scene, const GLuint material, const cone_options& opts) :
		generated_mesh(nullptr),
		m_material(material),
		m_options(opts)
	{
		recompute(scene);
	}
public:
	std::unordered_map<std::string, generated_mesh_param> get_params() const override
	{
		return {
			{ "Steps", { false, false, (void*)&m_options.num_steps, 3.f, 64.f, 1.f } },
		};
	}
	void recompute(scene_ctx* const scene) override
	{
		// delete mesh;
		mesh = scene->create_textured_cone(m_material, m_options);
	}
	virtual generated_mesh* clone() const
	{
		return new generated_cone(m_material, m_options);
	}
private:
	generated_cone(const GLuint material, const cone_options& opts) :
		generated_mesh(nullptr),
		m_material(material),
		m_options(opts)
	{}
private:
	GLuint m_material;
	cone_options m_options;
};

class generated_torus : public generated_mesh
{
public:
	generated_torus(scene_ctx* const scene, const GLuint material, const torus_options& opts) :
		generated_mesh(nullptr),
		m_material(material),
		m_options(opts)
	{
		recompute(scene);
	}
public:
	std::unordered_map<std::string, generated_mesh_param> get_params() const override
	{
		return {
			{ "Center Radius", { true, true, (void*)&m_options.center_radius, MIN_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
			{ "Tube Radius", { true, true, (void*)&m_options.tube_radius, MIN_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
			{ "Center Steps", { false, false, (void*)&m_options.num_center_steps, 3.f, 64.f, 1.f } },
			{ "Tube Steps", { false, false, (void*)&m_options.num_tube_steps, 3.f, 64.f, 1.f } },
		};
	}
	void recompute(scene_ctx* const scene) override
	{
		// delete mesh;
		mesh = scene->create_textured_torus(m_material, m_options);
	}
	virtual generated_mesh* clone() const
	{
		return new generated_torus(m_material, m_options);
	}
private:
	generated_torus(const GLuint material, const torus_options& opts) :
		generated_mesh(nullptr),
		m_material(material),
		m_options(opts)
	{}
private:
	GLuint m_material;
	torus_options m_options;
};

class generated_heightmap : public generated_mesh
{
public:
	generated_heightmap(scene_ctx* const scene, const GLuint material, const heightmap_options& opts) :
		generated_mesh(nullptr),
		m_material(material),
		m_options(opts)
	{
		recompute(scene);
	}
	std::unordered_map<std::string, generated_mesh_param> get_params() const override
	{
		// TODO
		assert(false);
		return {};
	}
	void recompute(scene_ctx* const scene) override
	{
		// delete mesh;
		// TODO
		assert(false);
	}
	virtual generated_mesh* clone() const
	{
		// TODO
		assert(false);
		return new generated_heightmap(m_material, m_options);
	}
private:
	generated_heightmap(const GLuint material, const heightmap_options& opts) :
		generated_mesh(nullptr),
		m_material(material),
		m_options(opts)
	{}
private:
	GLuint m_material;
	heightmap_options m_options;
};
