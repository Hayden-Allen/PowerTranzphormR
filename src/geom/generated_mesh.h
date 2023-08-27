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
	static generated_mesh* create(const nlohmann::json& obj);
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
	virtual nlohmann::json save() const
	{
		return {};
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
	generated_cuboid(const nlohmann::json& obj) :
		generated_mesh(nullptr),
		m_material(obj["mat"])
	{
		const auto& opts = obj["opts"];
		m_options.width = opts["w"];
		m_options.height = opts["h"];
		m_options.depth = opts["d"];
		m_options.transform = json2tmat<space::OBJECT, space::PARENT>(opts["t"]);
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
	nlohmann::json save() const override
	{
		nlohmann::json obj;
		obj["type"] = 0;
		obj["mat"] = m_material;
		obj["opts"] = {
			{ "w", m_options.width },
			{ "h", m_options.height },
			{ "d", m_options.depth },
			{ "t", m_options.transform.e },
		};
		return obj;
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
	generated_ellipsoid(const nlohmann::json& obj) :
		generated_mesh(nullptr),
		m_material(obj["mat"])
	{
		const auto& opts = obj["opts"];
		m_options.radius_x = opts["rx"];
		m_options.radius_y = opts["ry"];
		m_options.radius_z = opts["rz"];
		m_options.num_horizontal_steps = opts["nh"];
		m_options.num_vertical_steps = opts["nv"];
		m_options.transform = json2tmat<space::OBJECT, space::PARENT>(opts["t"]);
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
	nlohmann::json save() const override
	{
		nlohmann::json obj;
		obj["type"] = 1;
		obj["mat"] = m_material;
		obj["opts"] = {
			{ "rx", m_options.radius_x },
			{ "ry", m_options.radius_y },
			{ "rz", m_options.radius_z },
			{ "nh", m_options.num_horizontal_steps },
			{ "nv", m_options.num_vertical_steps },
			{ "t", m_options.transform.e },
		};
		return obj;
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
	generated_cylinder(const nlohmann::json& obj) :
		generated_mesh(nullptr),
		m_material(obj["mat"])
	{
		const auto& opts = obj["opts"];
		m_options.top_radius = opts["rt"];
		m_options.bottom_radius = opts["rb"];
		m_options.height = opts["h"];
		m_options.num_steps = opts["n"];
		m_options.transform = json2tmat<space::OBJECT, space::PARENT>(opts["t"]);
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
	nlohmann::json save() const override
	{
		nlohmann::json obj;
		obj["type"] = 2;
		obj["mat"] = m_material;
		obj["opts"] = {
			{ "rt", m_options.top_radius },
			{ "rb", m_options.bottom_radius },
			{ "h", m_options.height },
			{ "n", m_options.num_steps },
			{ "t", m_options.transform.e },
		};
		return obj;
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
	generated_cone(const nlohmann::json& obj) :
		generated_mesh(nullptr),
		m_material(obj["mat"])
	{
		const auto& opts = obj["opts"];
		m_options.radius = opts["r"];
		m_options.height = opts["h"];
		m_options.num_steps = opts["n"];
		m_options.transform = json2tmat<space::OBJECT, space::PARENT>(opts["t"]);
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
	nlohmann::json save() const override
	{
		nlohmann::json obj;
		obj["type"] = 3;
		obj["mat"] = m_material;
		obj["opts"] = {
			{ "r", m_options.radius },
			{ "h", m_options.height },
			{ "n", m_options.num_steps },
			{ "t", m_options.transform.e },
		};
		return obj;
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
	generated_torus(const nlohmann::json& obj) :
		generated_mesh(nullptr),
		m_material(obj["mat"])
	{
		const auto& opts = obj["opts"];
		m_options.center_radius = opts["rc"];
		m_options.tube_radius = opts["rt"];
		m_options.num_center_steps = opts["nc"];
		m_options.num_tube_steps = opts["nt"];
		m_options.transform = json2tmat<space::OBJECT, space::PARENT>(opts["t"]);
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
	nlohmann::json save() const override
	{
		nlohmann::json obj;
		obj["type"] = 4;
		obj["mat"] = m_material;
		obj["opts"] = {
			{ "rc", m_options.center_radius },
			{ "rt", m_options.tube_radius },
			{ "nc", m_options.num_center_steps },
			{ "nt", m_options.num_tube_steps },
			{ "t", m_options.transform.e },
		};
		return obj;
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
	generated_heightmap(const nlohmann::json& obj) :
		generated_mesh(nullptr),
		m_material(obj["mat"])
	{
		assert(false);
		const auto& opts = obj["opts"];
		m_options.width = opts["w"];
		m_options.max_height = opts["mh"];
		m_options.depth = opts["d"];
		m_options.width_steps = opts["nw"];
		m_options.depth_steps = opts["nd"];
		m_options.transform = json2tmat<space::OBJECT, space::PARENT>(opts["t"]);
	}
public:
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
	nlohmann::json save() const override
	{
		// TODO
		assert(false);
		nlohmann::json obj;
		obj["type"] = 5;
		obj["mat"] = m_material;
		obj["opts"] = {
			{ "w", m_options.width },
			{ "mh", m_options.max_height },
			{ "d", m_options.depth },
			{ "nw", m_options.width_steps },
			{ "nd", m_options.depth_steps },
			{ "t", m_options.transform.e },
		};
		return obj;
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
