#include "pch.h"
#include "generated_mesh.h"

generated_mesh::generated_mesh(mesh_t* const m) :
	mesh(m),
	dirty(true)
{}
generated_mesh::~generated_mesh() {}
generated_mesh* generated_mesh::create(const nlohmann::json& obj)
{
	const u32 type = obj["type"];
	switch (type)
	{
	case 0: return new generated_cuboid(obj);
	case 1: return new generated_ellipsoid(obj);
	case 2: return new generated_cylinder(obj);
	case 3: return new generated_cone(obj);
	case 4: return new generated_torus(obj);
	case 5: return new generated_heightmap(obj);
	}
	assert(false);
	return nullptr;
}
std::unordered_map<std::string, generated_mesh_param> generated_mesh::get_params() const
{
	return {};
}
void generated_mesh::recompute(scene_ctx* const scene) {}
generated_mesh* generated_mesh::clone() const
{
	return new generated_mesh(nullptr);
}
nlohmann::json generated_mesh::save() const
{
	return {};
}



generated_cuboid::generated_cuboid(scene_ctx* const scene, const GLuint material, const cuboid_options& opts) :
	generated_mesh(nullptr),
	m_material(material),
	m_options(opts)
{
	recompute(scene);
}
generated_cuboid::generated_cuboid(const nlohmann::json& obj) :
	generated_mesh(nullptr),
	m_material(obj["mat"])
{
	const auto& opts = obj["opts"];
	m_options.width = opts["w"];
	m_options.height = opts["h"];
	m_options.depth = opts["d"];
	m_options.transform = json2tmat<space::OBJECT, space::PARENT>(opts["t"]);
}
std::unordered_map<std::string, generated_mesh_param> generated_cuboid::get_params() const
{
	return {};
}
void generated_cuboid::recompute(scene_ctx* const scene)
{
	// delete mesh;
	mesh = scene->create_textured_cuboid(m_material, m_options);
}
generated_mesh* generated_cuboid::clone() const
{
	return new generated_cuboid(m_material, m_options);
}
nlohmann::json generated_cuboid::save() const
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
generated_cuboid::generated_cuboid(const GLuint material, const cuboid_options& opts) :
	generated_mesh(nullptr),
	m_material(material),
	m_options(opts)
{}



generated_ellipsoid::generated_ellipsoid(scene_ctx* const scene, const GLuint material, const ellipsoid_options& opts) :
	generated_mesh(nullptr),
	m_material(material),
	m_options(opts)
{
	recompute(scene);
}
generated_ellipsoid::generated_ellipsoid(const nlohmann::json& obj) :
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
std::unordered_map<std::string, generated_mesh_param> generated_ellipsoid::get_params() const
{
	return {
		{ "X Steps", { false, false, (void*)&m_options.num_horizontal_steps, 3.f, 64.f, 1.f } },
		{ "Y Steps", { false, false, (void*)&m_options.num_vertical_steps, 3.f, 64.f, 1.f } },
	};
}
void generated_ellipsoid::recompute(scene_ctx* const scene)
{
	// delete mesh;
	mesh = scene->create_textured_ellipsoid(m_material, m_options);
}
generated_mesh* generated_ellipsoid::clone() const
{
	return new generated_ellipsoid(m_material, m_options);
}
nlohmann::json generated_ellipsoid::save() const
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
generated_ellipsoid::generated_ellipsoid(const GLuint material, const ellipsoid_options& opts) :
	generated_mesh(nullptr),
	m_material(material),
	m_options(opts)
{}



generated_cylinder::generated_cylinder(scene_ctx* const scene, const GLuint material, const cylinder_options& opts) :
	generated_mesh(nullptr),
	m_material(material),
	m_options(opts)
{
	recompute(scene);
}
generated_cylinder::generated_cylinder(const nlohmann::json& obj) :
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
std::unordered_map<std::string, generated_mesh_param> generated_cylinder::get_params() const
{
	return {
		{ "Steps", { false, false, (void*)&m_options.num_steps, 3.f, 64.f, 1.f } },
		{ "Top Radius", { true, true, (void*)&m_options.top_radius, MIN_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
		{ "Bottom Radius", { true, true, (void*)&m_options.bottom_radius, MIN_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
	};
}
void generated_cylinder::recompute(scene_ctx* const scene)
{
	// delete mesh;
	mesh = scene->create_textured_cylinder(m_material, m_options);
}
generated_mesh* generated_cylinder::clone() const
{
	return new generated_cylinder(m_material, m_options);
}
nlohmann::json generated_cylinder::save() const
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
generated_cylinder::generated_cylinder(const GLuint material, const cylinder_options& opts) :
	generated_mesh(nullptr),
	m_material(material),
	m_options(opts)
{}



generated_cone::generated_cone(scene_ctx* const scene, const GLuint material, const cone_options& opts) :
	generated_mesh(nullptr),
	m_material(material),
	m_options(opts)
{
	recompute(scene);
}
generated_cone::generated_cone(const nlohmann::json& obj) :
	generated_mesh(nullptr),
	m_material(obj["mat"])
{
	const auto& opts = obj["opts"];
	m_options.radius = opts["r"];
	m_options.height = opts["h"];
	m_options.num_steps = opts["n"];
	m_options.transform = json2tmat<space::OBJECT, space::PARENT>(opts["t"]);
}
std::unordered_map<std::string, generated_mesh_param> generated_cone::get_params() const
{
	return {
		{ "Steps", { false, false, (void*)&m_options.num_steps, 3.f, 64.f, 1.f } },
	};
}
void generated_cone::recompute(scene_ctx* const scene)
{
	// delete mesh;
	mesh = scene->create_textured_cone(m_material, m_options);
}
generated_mesh* generated_cone::clone() const
{
	return new generated_cone(m_material, m_options);
}
nlohmann::json generated_cone::save() const
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
generated_cone::generated_cone(const GLuint material, const cone_options& opts) :
	generated_mesh(nullptr),
	m_material(material),
	m_options(opts)
{}



generated_torus::generated_torus(scene_ctx* const scene, const GLuint material, const torus_options& opts) :
	generated_mesh(nullptr),
	m_material(material),
	m_options(opts)
{
	recompute(scene);
}
generated_torus::generated_torus(const nlohmann::json& obj) :
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
std::unordered_map<std::string, generated_mesh_param> generated_torus::get_params() const
{
	return {
		{ "Center Radius", { true, true, (void*)&m_options.center_radius, MIN_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
		{ "Tube Radius", { true, true, (void*)&m_options.tube_radius, MIN_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
		{ "Center Steps", { false, false, (void*)&m_options.num_center_steps, 3.f, 64.f, 1.f } },
		{ "Tube Steps", { false, false, (void*)&m_options.num_tube_steps, 3.f, 64.f, 1.f } },
	};
}
void generated_torus::recompute(scene_ctx* const scene)
{
	// delete mesh;
	mesh = scene->create_textured_torus(m_material, m_options);
}
generated_mesh* generated_torus::clone() const
{
	return new generated_torus(m_material, m_options);
}
nlohmann::json generated_torus::save() const
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
generated_torus::generated_torus(const GLuint material, const torus_options& opts) :
	generated_mesh(nullptr),
	m_material(material),
	m_options(opts)
{}



generated_heightmap::generated_heightmap(scene_ctx* const scene, const GLuint material, const heightmap_options& opts) :
	generated_mesh(nullptr),
	m_material(material),
	m_options(opts)
{
	recompute(scene);
}
generated_heightmap::generated_heightmap(const nlohmann::json& obj) :
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
std::unordered_map<std::string, generated_mesh_param> generated_heightmap::get_params() const
{
	// TODO
	assert(false);
	return {};
}
void generated_heightmap::recompute(scene_ctx* const scene)
{
	// delete mesh;
	// TODO
	assert(false);
}
generated_mesh* generated_heightmap::clone() const
{
	// TODO
	assert(false);
	return new generated_heightmap(m_material, m_options);
}
nlohmann::json generated_heightmap::save() const
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
generated_heightmap::generated_heightmap(const GLuint material, const heightmap_options& opts) :
	generated_mesh(nullptr),
	m_material(material),
	m_options(opts)
{}
