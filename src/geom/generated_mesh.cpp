#include "pch.h"
#include "generated_mesh.h"

generated_mesh::generated_mesh(mesh_t* const m) :
	mesh(m),
	dirty(true)
{
	copy_verts();
}
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
generated_mesh* generated_mesh::clone(const tmat<space::OBJECT, space::WORLD>& old_transform) const
{
	const auto& inv = old_transform.invert_copy().cast_copy<space::OBJECT, space::OBJECT>();
	size_t i = 0;
	mesh->transform([&](vertex_t::vector_t& v)
		{
			const auto& out = hats2carve(src_verts[i].transform_copy(inv));
			++i;
			return out;
		});
	return new generated_mesh(mesh);
}
nlohmann::json generated_mesh::save() const
{
	return {};
}
GLuint generated_mesh::get_material() const
{
	assert(false);
	GLuint tmp = 0;
	return MAX_VALUE(tmp);
}
void generated_mesh::set_material(const GLuint mat)
{
	assert(false);
}
primitive_options* generated_mesh::get_options() const
{
	assert(false);
	return nullptr;
}
void generated_mesh::copy_verts()
{
	src_verts.clear();
	if (mesh)
	{
		for (const auto& v : mesh->vertex_storage)
		{
			src_verts.emplace_back(point<space::OBJECT>(v.v.x, v.v.y, v.v.z));
		}
	}
}



generated_primitive::generated_primitive(mesh_t* const m, const GLuint material) :
	generated_mesh(m),
	m_material(material)
{}
generated_primitive::generated_primitive(const nlohmann::json& obj) :
	generated_mesh(nullptr),
	m_material(obj["mat"])
{}
generated_primitive::~generated_primitive() {}
std::unordered_map<std::string, generated_mesh_param> generated_primitive::get_params() const
{
	const primitive_options* const opts = get_options();
	return {
		{ "Material", { false, (void*)&m_material, 0.f, 1.f * MAX_VALUE(m_material), 1.f } },
		{ "U Scale", { true, (void*)&opts->u_scale, MIN_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
		{ "V Scale", { true, (void*)&opts->v_scale, MIN_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
	};
}
void generated_primitive::recompute(scene_ctx* const scene)
{
	copy_verts();
}
GLuint generated_primitive::get_material() const
{
	return m_material;
}
void generated_primitive::set_material(const GLuint mat)
{
	m_material = mat;
	dirty = true;
}
nlohmann::json generated_primitive::save() const
{
	const primitive_options* const opts = get_options();
	nlohmann::json obj;
	obj["opts"] = {
		{ "t", opts->transform.e },
		{ "u", opts->u_scale },
		{ "v", opts->v_scale },
	};
	return obj;
}



generated_cuboid::generated_cuboid(scene_ctx* const scene, const GLuint material, const cuboid_options& opts) :
	generated_primitive(nullptr, material),
	m_options(opts)
{
	recompute(scene);
}
generated_cuboid::generated_cuboid(const nlohmann::json& obj) :
	generated_primitive(obj)
{
	const auto& opts = obj["opts"];
	m_options.transform = json2tmat<space::OBJECT, space::PARENT>(opts["t"]);
}
std::unordered_map<std::string, generated_mesh_param> generated_cuboid::get_params() const
{
	return generated_primitive::get_params();
}
void generated_cuboid::recompute(scene_ctx* const scene)
{
	// delete mesh;
	mesh = scene->create_textured_cuboid(m_material, m_options);
	generated_primitive::recompute(scene);
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
	obj["opts"] = generated_primitive::save()["opts"];
	return obj;
}
primitive_options* generated_cuboid::get_options() const
{
	return (primitive_options*)&m_options;
}
generated_cuboid::generated_cuboid(const GLuint material, const cuboid_options& opts) :
	generated_primitive(nullptr, material),
	m_options(opts)
{}



generated_ellipsoid::generated_ellipsoid(scene_ctx* const scene, const GLuint material, const ellipsoid_options& opts) :
	generated_primitive(nullptr, material),
	m_options(opts)
{
	recompute(scene);
}
generated_ellipsoid::generated_ellipsoid(const nlohmann::json& obj) :
	generated_primitive(obj)
{
	const auto& opts = obj["opts"];
	m_options.num_horizontal_steps = opts["nh"];
	m_options.num_vertical_steps = opts["nv"];
	m_options.transform = json2tmat<space::OBJECT, space::PARENT>(opts["t"]);
}
std::unordered_map<std::string, generated_mesh_param> generated_ellipsoid::get_params() const
{
	auto m = generated_primitive::get_params();
	std::unordered_map<std::string, generated_mesh_param> t = {
		{ "X Steps", { false, (void*)&m_options.num_horizontal_steps, 3.f, 64.f, 1.f } },
		{ "Y Steps", { false, (void*)&m_options.num_vertical_steps, 3.f, 64.f, 1.f } },
	};
	m.merge(t);
	return m;
}
void generated_ellipsoid::recompute(scene_ctx* const scene)
{
	// delete mesh;
	mesh = scene->create_textured_ellipsoid(m_material, m_options);
	generated_primitive::recompute(scene);
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
	obj["opts"] = generated_primitive::save()["opts"];
	obj["opts"]["nh"] = m_options.num_horizontal_steps;
	obj["opts"]["nv"] = m_options.num_vertical_steps;
	return obj;
}
primitive_options* generated_ellipsoid::get_options() const
{
	return (primitive_options*)&m_options;
}
generated_ellipsoid::generated_ellipsoid(const GLuint material, const ellipsoid_options& opts) :
	generated_primitive(nullptr, material),
	m_options(opts)
{}



generated_cylinder::generated_cylinder(scene_ctx* const scene, const GLuint material, const cylinder_options& opts) :
	generated_primitive(nullptr, material),
	m_options(opts)
{
	recompute(scene);
}
generated_cylinder::generated_cylinder(const nlohmann::json& obj) :
	generated_primitive(obj)
{
	const auto& opts = obj["opts"];
	m_options.top_radius = opts["rt"];
	m_options.bottom_radius = opts["rb"];
	m_options.num_steps = opts["n"];
	m_options.transform = json2tmat<space::OBJECT, space::PARENT>(opts["t"]);
}
std::unordered_map<std::string, generated_mesh_param> generated_cylinder::get_params() const
{
	auto m = generated_primitive::get_params();
	std::unordered_map<std::string, generated_mesh_param> t = {
		{ "Steps", { false, (void*)&m_options.num_steps, 3.f, 64.f, 1.f } },
		{ "Top Radius", { true, (void*)&m_options.top_radius, MIN_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
		{ "Bottom Radius", { true, (void*)&m_options.bottom_radius, MIN_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
	};
	m.merge(t);
	return m;
}
void generated_cylinder::recompute(scene_ctx* const scene)
{
	// delete mesh;
	mesh = scene->create_textured_cylinder(m_material, m_options);
	generated_primitive::recompute(scene);
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
	obj["opts"] = generated_primitive::save()["opts"];
	obj["opts"]["rt"] = m_options.top_radius;
	obj["opts"]["rb"] = m_options.bottom_radius;
	obj["opts"]["n"] = m_options.num_steps;
	return obj;
}
primitive_options* generated_cylinder::get_options() const
{
	return (primitive_options*)&m_options;
}
generated_cylinder::generated_cylinder(const GLuint material, const cylinder_options& opts) :
	generated_primitive(nullptr, material),
	m_options(opts)
{}



generated_cone::generated_cone(scene_ctx* const scene, const GLuint material, const cone_options& opts) :
	generated_primitive(nullptr, material),
	m_options(opts)
{
	recompute(scene);
}
generated_cone::generated_cone(const nlohmann::json& obj) :
	generated_primitive(obj)
{
	const auto& opts = obj["opts"];
	m_options.num_steps = opts["n"];
	m_options.transform = json2tmat<space::OBJECT, space::PARENT>(opts["t"]);
}
std::unordered_map<std::string, generated_mesh_param> generated_cone::get_params() const
{
	auto m = generated_primitive::get_params();
	std::unordered_map<std::string, generated_mesh_param> t = {
		{ "Steps", { false, (void*)&m_options.num_steps, 3.f, 64.f, 1.f } },
	};
	m.merge(t);
	return m;
}
void generated_cone::recompute(scene_ctx* const scene)
{
	// delete mesh;
	mesh = scene->create_textured_cone(m_material, m_options);
	generated_primitive::recompute(scene);
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
	obj["opts"] = generated_primitive::save()["opts"];
	obj["opts"]["n"] = m_options.num_steps;
	return obj;
}
primitive_options* generated_cone::get_options() const
{
	return (primitive_options*)&m_options;
}
generated_cone::generated_cone(const GLuint material, const cone_options& opts) :
	generated_primitive(nullptr, material),
	m_options(opts)
{}



generated_torus::generated_torus(scene_ctx* const scene, const GLuint material, const torus_options& opts) :
	generated_primitive(nullptr, material),
	m_options(opts)
{
	recompute(scene);
}
generated_torus::generated_torus(const nlohmann::json& obj) :
	generated_primitive(obj)
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
	auto m = generated_primitive::get_params();
	std::unordered_map<std::string, generated_mesh_param> t = {
		{ "Center Radius", { true, (void*)&m_options.center_radius, MIN_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
		{ "Tube Radius", { true, (void*)&m_options.tube_radius, MIN_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
		{ "Center Steps", { false, (void*)&m_options.num_center_steps, 3.f, 64.f, 1.f } },
		{ "Tube Steps", { false, (void*)&m_options.num_tube_steps, 3.f, 64.f, 1.f } },
	};
	m.merge(t);
	return m;
}
void generated_torus::recompute(scene_ctx* const scene)
{
	// delete mesh;
	mesh = scene->create_textured_torus(m_material, m_options);
	generated_primitive::recompute(scene);
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
	obj["opts"] = generated_primitive::save()["opts"];
	obj["opts"]["rc"] = m_options.center_radius;
	obj["opts"]["rt"] = m_options.tube_radius;
	obj["opts"]["nc"] = m_options.num_center_steps;
	obj["opts"]["nt"] = m_options.num_tube_steps;
	return obj;
}
primitive_options* generated_torus::get_options() const
{
	return (primitive_options*)&m_options;
}
generated_torus::generated_torus(const GLuint material, const torus_options& opts) :
	generated_primitive(nullptr, material),
	m_options(opts)
{}



generated_heightmap::generated_heightmap(scene_ctx* const scene, const GLuint material, const heightmap_options& opts) :
	generated_primitive(nullptr, material),
	m_options(opts)
{
	recompute(scene);
}
generated_heightmap::generated_heightmap(const nlohmann::json& obj) :
	generated_primitive(obj)
{
	assert(false);
	const auto& opts = obj["opts"];
	m_options.width_steps = opts["nw"];
	m_options.depth_steps = opts["nd"];
	m_options.transform = json2tmat<space::OBJECT, space::PARENT>(opts["t"]);
}
std::unordered_map<std::string, generated_mesh_param> generated_heightmap::get_params() const
{
	// TODO
	assert(false);
	return generated_primitive::get_params();
}
void generated_heightmap::recompute(scene_ctx* const scene)
{
	// delete mesh;
	// TODO
	assert(false);
	generated_primitive::recompute(scene);
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
	obj["opts"] = generated_primitive::save()["opts"];
	obj["opts"]["nw"] = m_options.width_steps;
	obj["opts"]["nd"] = m_options.depth_steps;
	return obj;
}
primitive_options* generated_heightmap::get_options() const
{
	return (primitive_options*)&m_options;
}
generated_heightmap::generated_heightmap(const GLuint material, const heightmap_options& opts) :
	generated_primitive(nullptr, material),
	m_options(opts)
{}
