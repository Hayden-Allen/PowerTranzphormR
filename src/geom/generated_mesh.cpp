#include "pch.h"
#include "generated_mesh.h"
#include "core/scene_ctx.h"

generated_mesh::generated_mesh(mesh_t* const m) :
	mesh(m),
	dirty(false)
{}
generated_mesh::~generated_mesh()
{
	delete mesh;
}
generated_mesh* generated_mesh::create(const nlohmann::json& obj, scene_ctx* const scene)
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
	case 6: return new generated_static_mesh(obj, scene);
	}
	assert(false);
	return nullptr;
}
std::vector<std::pair<std::string, generated_mesh_param>> generated_mesh::get_params() const
{
	return {};
}
void generated_mesh::recompute(scene_ctx* const scene)
{
	dirty = false;
}
generated_mesh* generated_mesh::clone(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& inv_mat) const
{
	return new generated_mesh(nullptr);
}
nlohmann::json generated_mesh::save(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& mat) const
{
	return {};
}
GLuint generated_mesh::get_material() const
{
	assert(false);
	GLuint tmp = 0;
	return MAX_VALUE(tmp);
}
void generated_mesh::set_material(scene_ctx* const scene, const GLuint mat)
{
	assert(false);
}
void generated_mesh::replace_material(scene_ctx* const scene, const GLuint old_mat, const GLuint new_mat)
{
	assert(false);
}
primitive_options* generated_mesh::get_options() const
{
	assert(false);
	return nullptr;
}
void generated_mesh::set_mesh(mesh_t* const m)
{
	assert(!mesh);
	mesh = m;
}
void generated_mesh::set_vertices_color(const color_t& col, scene_ctx* const scene)
{
	assert(false);
}
void generated_mesh::center_verts_at_origin(const tmat<space::OBJECT, space::WORLD>& mat)
{
	assert(false);
}
void generated_mesh::set_dirty()
{
	dirty = true;
}
bool generated_mesh::is_dirty() const
{
	return dirty;
}
void generated_mesh::clear()
{
	delete mesh;
	mesh = nullptr;
}
void generated_mesh::copy_mesh_from(const generated_mesh* const other, scene_ctx* const scene)
{
	set_mesh(carve_clone(other->mesh, scene));
}
mesh_t* generated_mesh::clone_mesh_to_local(scene_ctx* const scene, const tmat<space::OBJECT, space::WORLD>& mat) const
{
	const auto& inv = mat.invert_copy();
	mesh_t* const clone = carve_clone(mesh, scene);
	clone->transform([&](vertex_t::vector_t& v)
		{
			return u::hats2carve(point<space::WORLD>(v.x, v.y, v.z).transform_copy(inv));
		});
	return clone;
}
bool generated_mesh::is_static() const
{
	return false;
}
const tex_coord_t* generated_mesh::get_uv_offset() const
{
	return nullptr;
}



generated_primitive::generated_primitive(mesh_t* const m, const GLuint material) :
	generated_mesh(m),
	m_material(material)
{
	dirty = true;
}
generated_primitive::generated_primitive(const nlohmann::json& obj) :
	generated_mesh(nullptr),
	m_material(obj["mat"])
{}
generated_primitive::~generated_primitive() {}
std::vector<std::pair<std::string, generated_mesh_param>> generated_primitive::get_params() const
{
	const primitive_options* const opts = get_options();
	return {
		{ "UV0", { generated_mesh_param_type::FLOAT_4, (void*)&opts->u0, -MAX_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
		{ "UV1", { generated_mesh_param_type::FLOAT_4, (void*)&opts->u1, -MAX_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
		{ "UV2", { generated_mesh_param_type::FLOAT_4, (void*)&opts->u2, -MAX_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
		{ "UV3", { generated_mesh_param_type::FLOAT_4, (void*)&opts->u3, -MAX_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
		{ "Weights", { generated_mesh_param_type::FLOAT_4_SUM_1, (void*)&opts->w0, 0.0f, 1.0f, DRAG_PARAM_STEP } },
		{ "Color", { generated_mesh_param_type::COLOR_4, (void*)&opts->r, 0.f, 1.f, DRAG_PARAM_STEP } },
	};
}
void generated_primitive::recompute(scene_ctx* const scene)
{
	generated_mesh::recompute(scene);
	delete mesh;
}
GLuint generated_primitive::get_material() const
{
	return m_material;
}
void generated_primitive::set_material(scene_ctx* const scene, const GLuint mat)
{
	m_material = mat;
	dirty = true;
}
void generated_primitive::replace_material(scene_ctx* const scene, const GLuint old_mat, const GLuint new_mat)
{
	if (m_material == old_mat)
	{
		m_material = new_mat;
		dirty = true;
	}
}
nlohmann::json generated_primitive::save(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& mat) const
{
	const primitive_options* const opts = get_options();
	nlohmann::json obj;
	obj["opts"] = {
		// { "t", opts->transform.e },
		{ "uv0", { opts->u0, opts->v0, opts->uo0, opts->vo0 } },
		{ "uv1", { opts->u1, opts->v1, opts->uo1, opts->vo1 } },
		{ "uv2", { opts->u2, opts->v2, opts->uo2, opts->vo2 } },
		{ "uv3", { opts->u3, opts->v3, opts->uo3, opts->vo3 } },
		{ "w", { opts->w0, opts->w1, opts->w2, opts->w3 } },
		{ "c", { opts->r, opts->g, opts->b, opts->a } },
	};
	return obj;
}
void generated_primitive::set_options(const nlohmann::json& obj)
{
	primitive_options* const options = get_options();
	const auto& opts = obj["opts"];
	// options->transform = u::json2tmat<space::OBJECT, space::PARENT>(opts["t"]);
	options->u0 = opts["uv0"][0];
	options->v0 = opts["uv0"][1];
	options->uo0 = opts["uv0"][2];
	options->vo0 = opts["uv0"][3];
	options->u1 = opts["uv1"][0];
	options->v1 = opts["uv1"][1];
	options->uo1 = opts["uv1"][2];
	options->vo1 = opts["uv1"][3];
	options->u2 = opts["uv2"][0];
	options->v2 = opts["uv2"][1];
	options->uo2 = opts["uv2"][2];
	options->vo2 = opts["uv2"][3];
	options->u3 = opts["uv3"][0];
	options->v3 = opts["uv3"][1];
	options->uo3 = opts["uv3"][2];
	options->vo3 = opts["uv3"][3];
	options->w0 = opts["w"][0];
	options->w1 = opts["w"][1];
	options->w2 = opts["w"][2];
	options->w3 = opts["w"][3];
	options->r = opts["c"][0];
	options->g = opts["c"][1];
	options->b = opts["c"][2];
	options->a = opts["c"][3];
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
	generated_primitive::set_options(obj);
}
std::vector<std::pair<std::string, generated_mesh_param>> generated_cuboid::get_params() const
{
	return generated_primitive::get_params();
}
void generated_cuboid::recompute(scene_ctx* const scene)
{
	generated_primitive::recompute(scene);
	mesh = scene->create_textured_cuboid(m_material, m_options);
}
generated_mesh* generated_cuboid::clone(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& inv_mat) const
{
	return new generated_cuboid(m_material, m_options);
}
nlohmann::json generated_cuboid::save(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& mat) const
{
	nlohmann::json obj;
	obj["type"] = 0;
	obj["mat"] = m_material;
	obj["opts"] = generated_primitive::save(scene, mat)["opts"];
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
	generated_primitive::set_options(obj);
	const auto& opts = obj["opts"];
	m_options.num_horizontal_steps = opts["nh"];
	m_options.num_vertical_steps = opts["nv"];
}
std::vector<std::pair<std::string, generated_mesh_param>> generated_ellipsoid::get_params() const
{
	auto m = generated_primitive::get_params();
	std::vector<std::pair<std::string, generated_mesh_param>> t = {
		{ "XY Steps", { generated_mesh_param_type::UINT_2_LOG, (void*)&m_options.num_horizontal_steps, 3.f, 64.f, 1.f } },
		{ "Noise", { generated_mesh_param_type::FLOAT_1, (void*)&m_options.noise, 0.f, .3f, .001f } }
	};
	m.insert(m.end(), t.begin(), t.end());
	return m;
}
void generated_ellipsoid::recompute(scene_ctx* const scene)
{
	generated_primitive::recompute(scene);
	mesh = scene->create_textured_ellipsoid(m_material, m_options);
}
generated_mesh* generated_ellipsoid::clone(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& inv_mat) const
{
	return new generated_ellipsoid(m_material, m_options);
}
nlohmann::json generated_ellipsoid::save(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& mat) const
{
	nlohmann::json obj;
	obj["type"] = 1;
	obj["mat"] = m_material;
	obj["opts"] = generated_primitive::save(scene, mat)["opts"];
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
	generated_primitive::set_options(obj);
	const auto& opts = obj["opts"];
	m_options.top_radius = opts["rt"];
	m_options.bottom_radius = opts["rb"];
	m_options.num_steps = opts["n"];
}
std::vector<std::pair<std::string, generated_mesh_param>> generated_cylinder::get_params() const
{
	auto m = generated_primitive::get_params();
	std::vector<std::pair<std::string, generated_mesh_param>> t = {
		{ "Steps", { generated_mesh_param_type::UINT_1_LOG, (void*)&m_options.num_steps, 3.f, 64.f, 1.f } },
		{ "Top Radius", { generated_mesh_param_type::FLOAT_1, (void*)&m_options.top_radius, MIN_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
		{ "Bottom Radius", { generated_mesh_param_type::FLOAT_1, (void*)&m_options.bottom_radius, MIN_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
	};
	m.insert(m.end(), t.begin(), t.end());
	return m;
}
void generated_cylinder::recompute(scene_ctx* const scene)
{
	generated_primitive::recompute(scene);
	mesh = scene->create_textured_cylinder(m_material, m_options);
}
generated_mesh* generated_cylinder::clone(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& inv_mat) const
{
	return new generated_cylinder(m_material, m_options);
}
nlohmann::json generated_cylinder::save(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& mat) const
{
	nlohmann::json obj;
	obj["type"] = 2;
	obj["mat"] = m_material;
	obj["opts"] = generated_primitive::save(scene, mat)["opts"];
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
	generated_primitive::set_options(obj);
	const auto& opts = obj["opts"];
	m_options.num_steps = opts["n"];
}
std::vector<std::pair<std::string, generated_mesh_param>> generated_cone::get_params() const
{
	auto m = generated_primitive::get_params();
	std::vector<std::pair<std::string, generated_mesh_param>> t = {
		{ "Steps", { generated_mesh_param_type::UINT_1_LOG, (void*)&m_options.num_steps, 3.f, 64.f, 1.f } },
	};
	m.insert(m.end(), t.begin(), t.end());
	return m;
}
void generated_cone::recompute(scene_ctx* const scene)
{
	generated_primitive::recompute(scene);
	mesh = scene->create_textured_cone(m_material, m_options);
}
generated_mesh* generated_cone::clone(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& inv_mat) const
{
	return new generated_cone(m_material, m_options);
}
nlohmann::json generated_cone::save(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& mat) const
{
	nlohmann::json obj;
	obj["type"] = 3;
	obj["mat"] = m_material;
	obj["opts"] = generated_primitive::save(scene, mat)["opts"];
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
	generated_primitive::set_options(obj);
	const auto& opts = obj["opts"];
	m_options.center_radius = opts["rc"];
	m_options.tube_radius = opts["rt"];
	m_options.num_center_steps = opts["nc"];
	m_options.num_tube_steps = opts["nt"];
}
std::vector<std::pair<std::string, generated_mesh_param>> generated_torus::get_params() const
{
	auto m = generated_primitive::get_params();
	std::vector<std::pair<std::string, generated_mesh_param>> t = {
		{ "Radii", { generated_mesh_param_type::FLOAT_2, (void*)&m_options.center_radius, MIN_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
		{ "Steps", { generated_mesh_param_type::UINT_2, (void*)&m_options.num_center_steps, 3.f, 64.f, 1.f } },
	};
	m.insert(m.end(), t.begin(), t.end());
	return m;
}
void generated_torus::recompute(scene_ctx* const scene)
{
	generated_primitive::recompute(scene);
	mesh = scene->create_textured_torus(m_material, m_options);
}
generated_mesh* generated_torus::clone(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& inv_mat) const
{
	return new generated_torus(m_material, m_options);
}
nlohmann::json generated_torus::save(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& mat) const
{
	nlohmann::json obj;
	obj["type"] = 4;
	obj["mat"] = m_material;
	obj["opts"] = generated_primitive::save(scene, mat)["opts"];
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
	generated_primitive::set_options(obj);
	const auto& opts = obj["opts"];
	m_options.width_steps = opts["nw"];
	m_options.depth_steps = opts["nd"];
}
std::vector<std::pair<std::string, generated_mesh_param>> generated_heightmap::get_params() const
{
	auto m = generated_primitive::get_params();
	std::vector<std::pair<std::string, generated_mesh_param>> t = {
		{ "Steps", { generated_mesh_param_type::UINT_2_DIRECT, (void*)&m_options.width_steps, 2, 64, 1 } },
	};
	m.insert(m.end(), t.begin(), t.end());
	return m;
}
void generated_heightmap::recompute(scene_ctx* const scene)
{
	generated_primitive::recompute(scene);
	mesh = scene->create_textured_heightmap(m_material, m_options);
}
generated_mesh* generated_heightmap::clone(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& inv_mat) const
{
	return new generated_heightmap(m_material, m_options);
}
nlohmann::json generated_heightmap::save(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& mat) const
{
	nlohmann::json obj;
	obj["type"] = 5;
	obj["mat"] = m_material;
	obj["opts"] = generated_primitive::save(scene, mat)["opts"];
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



generated_static_mesh::generated_static_mesh(mesh_t* const m, scene_ctx* const scene) :
	generated_mesh(m)
{
	for (u32 i = 0; i < 4; i++)
		m_uv_offset[i] = tex_coord_t(1, 1);
	check_material_id(scene);
	dirty = false;
}
generated_static_mesh::generated_static_mesh(const nlohmann::json& obj, scene_ctx* const scene) :
	generated_mesh(nullptr),
	m_material(obj["mat"])
{
	for (u32 i = 0; i < 4; i++)
	{
		m_uv_offset[i] = tex_coord_t(obj["uvo"][i][0], obj["uvo"][i][1], obj["uvo"][i][2], obj["uvo"][i][3]);
	}
	auto& mtl_id_attr = scene->get_mtl_id_attr();
	auto& vert_attrs = scene->get_vert_attrs();

	const nlohmann::json::array_t& overts = obj["v"];
	std::vector<vertex_t> verts;
	for (const nlohmann::json::array_t& overt : overts)
	{
		verts.emplace_back(carve::geom::VECTOR(overt.at(0), overt.at(1), overt.at(2)));
	}

	const nlohmann::json::array_t& ofaces = obj["f"];
	std::vector<face_t*> faces;
	for (const nlohmann::json& oface : ofaces)
	{
		const nlohmann::json::array_t& oface_verts = oface["v"];
		std::vector<vertex_t*> face_verts;
		face_verts.reserve(oface_verts.size());
		for (const nlohmann::json& oface_vert : oface_verts)
		{
			face_verts.push_back(verts.data() + oface_vert["v"]);
		}
		face_t* const face = new face_t(face_verts.begin(), face_verts.end());
		for (const nlohmann::json& oface_vert : oface_verts)
		{
			const u32 i = oface_vert["i"];
			const auto& uv0 = oface_vert["uv0"];
			const auto& uv1 = oface_vert["uv1"];
			const auto& uv2 = oface_vert["uv2"];
			const auto& uv3 = oface_vert["uv3"];
			vert_attrs.uv0.setAttribute(face, i, tex_coord_t(uv0[0], uv0[1], uv0[2], uv0[3]));
			vert_attrs.uv1.setAttribute(face, i, tex_coord_t(uv1[0], uv1[1], uv1[2], uv1[3]));
			vert_attrs.uv2.setAttribute(face, i, tex_coord_t(uv2[0], uv2[1], uv2[2], uv2[3]));
			vert_attrs.uv3.setAttribute(face, i, tex_coord_t(uv3[0], uv3[1], uv3[2], uv3[3]));
			const auto& w = oface_vert["w"];
			vert_attrs.w0.setAttribute(face, i, w[0]);
			vert_attrs.w1.setAttribute(face, i, w[1]);
			vert_attrs.w2.setAttribute(face, i, w[2]);
			vert_attrs.w3.setAttribute(face, i, w[3]);
			const auto& c = oface_vert["c"];
			vert_attrs.color.setAttribute(face, i, color_t(c[0], c[1], c[2], c[3]));
		}
		const u32 mtl_id = oface["m"];
		mtl_id_attr.setAttribute(face, mtl_id);
		faces.push_back(face);
	}

	mesh = new mesh_t(faces);

	check_material_id(scene);
}
void generated_static_mesh::set_material(scene_ctx* const scene, const GLuint new_mat)
{
	auto& mtl_id_attr = scene->get_mtl_id_attr();

	for (mesh_t::face_iter i = mesh->faceBegin(); i != mesh->faceEnd(); ++i)
	{
		const face_t* const f = *i;
		mtl_id_attr.setAttribute(f, new_mat);
	}

	check_material_id(scene);
}
void generated_static_mesh::replace_material(scene_ctx* const scene, const GLuint old_mat, const GLuint new_mat)
{
	auto& mtl_id_attr = scene->get_mtl_id_attr();

	for (mesh_t::face_iter i = mesh->faceBegin(); i != mesh->faceEnd(); ++i)
	{
		const face_t* const f = *i;
		const u32 old_mtl_id = mtl_id_attr.getAttribute(f, 0);
		if (old_mtl_id == old_mat)
		{
			mtl_id_attr.setAttribute(f, new_mat);
		}
	}

	check_material_id(scene);
}
generated_mesh* generated_static_mesh::clone(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& inv_mat) const
{
	return new generated_static_mesh(carve_clone(mesh, scene, inv_mat), scene);
}
nlohmann::json generated_static_mesh::save(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& mat) const
{
	auto& mtl_id_attr = scene->get_mtl_id_attr();
	// auto& tex_coord_attr = scene->get_tex_coord_attr();
	auto& vert_attrs = scene->get_vert_attrs();

	nlohmann::json obj;
	obj["type"] = 6;
	obj["mat"] = m_material;
	obj["uvo"] = nlohmann::json::array_t();
	for (u32 i = 0; i < 4; i++)
		obj["uvo"].push_back({ m_uv_offset[0].u, m_uv_offset[0].v, m_uv_offset[0].uo, m_uv_offset[0].vo });

	nlohmann::json::array_t verts;
	std::unordered_map<const vertex_t*, u64> vert2index;
	for (u64 i = 0; i < mesh->vertex_storage.size(); i++)
	{
		const mesh_t::vertex_t* const vert = &mesh->vertex_storage[i];
		const point<space::WORLD> p(vert->v.x, vert->v.y, vert->v.z);
		const point<space::OBJECT>& op = p.transform_copy(mat);
		nlohmann::json::array_t j;
		j.push_back(op.x);
		j.push_back(op.y);
		j.push_back(op.z);
		verts.push_back(j);
		vert2index.insert({ vert, i });
	}
	obj["v"] = verts;

	nlohmann::json::array_t faces;
	std::unordered_map<const vertex_t*, tex_coord_t> vert2tc;
	for (mesh_t::face_iter i = mesh->faceBegin(); i != mesh->faceEnd(); ++i)
	{
		const face_t* const f = *i;
		const u32 mtl_id = mtl_id_attr.getAttribute(f, 0);
		nlohmann::json face;
		face["m"] = mtl_id;
		nlohmann::json::array_t face_verts;
		for (face_t::const_edge_iter_t e = f->begin(); e != f->end(); ++e)
		{
			const tex_coord_t uv0 = vert_attrs.uv0.getAttribute(f, e.idx());
			const tex_coord_t uv1 = vert_attrs.uv1.getAttribute(f, e.idx());
			const tex_coord_t uv2 = vert_attrs.uv2.getAttribute(f, e.idx());
			const tex_coord_t uv3 = vert_attrs.uv3.getAttribute(f, e.idx());
			const f64 w0 = vert_attrs.w0.getAttribute(f, e.idx());
			const f64 w1 = vert_attrs.w1.getAttribute(f, e.idx());
			const f64 w2 = vert_attrs.w2.getAttribute(f, e.idx());
			const f64 w3 = vert_attrs.w3.getAttribute(f, e.idx());
			const color_t& color = vert_attrs.color.getAttribute(f, e.idx());

			const nlohmann::json face_vert = {
				{ "v", vert2index.at(e->vert) },
				{ "i", e.idx() },
				{ "uv0", { uv0.u, uv0.v, uv0.uo, uv0.vo } },
				{ "uv1", { uv1.u, uv1.v, uv1.uo, uv1.vo } },
				{ "uv2", { uv2.u, uv2.v, uv2.uo, uv2.vo } },
				{ "uv3", { uv3.u, uv3.v, uv3.uo, uv3.vo } },
				{ "w", { w0, w1, w2, w3 } },
				{ "c", { color.r, color.g, color.b, color.a } },
			};

			face_verts.push_back(face_vert);
		}
		face["v"] = face_verts;
		faces.push_back(face);
	}
	obj["f"] = faces;

	return obj;
}
bool generated_static_mesh::is_static() const
{
	return true;
}
void generated_static_mesh::set_mesh(mesh_t* const m)
{
	generated_mesh::set_mesh(m);
}
GLuint generated_static_mesh::get_material() const
{
	return m_material;
}
void generated_static_mesh::set_vertices_color(const color_t& col, scene_ctx* const scene)
{
	auto& vert_attrs = scene->get_vert_attrs();
	for (mesh_t::face_iter i = mesh->faceBegin(); i != mesh->faceEnd(); ++i)
	{
		const face_t* const f = *i;
		for (face_t::const_edge_iter_t e = f->begin(); e != f->end(); ++e)
		{
			vert_attrs.color.setAttribute(f, e.idx(), col);
		}
	}
}
void generated_static_mesh::center_verts_at_origin(const tmat<space::OBJECT, space::WORLD>& mat)
{
	const auto& inv_mat = mat.invert_copy();
	point<space::OBJECT> min_point, max_point;
	mesh->transform([&](vertex_t::vector_t& v)
		{
			const auto& p = point<space::WORLD>(v.x, v.y, v.z).transform_copy(inv_mat);
			if (p.x < min_point.x)
				min_point.x = p.x;
			if (p.y < min_point.y)
				min_point.y = p.y;
			if (p.z < min_point.z)
				min_point.z = p.z;
			if (p.x > max_point.x)
				max_point.x = p.x;
			if (p.y > max_point.y)
				max_point.y = p.y;
			if (p.z > max_point.z)
				max_point.z = p.z;
			return v;
		});
	point<space::OBJECT> avg_point((max_point.x + min_point.x) * 0.5f, (max_point.y + min_point.y) * 0.5f, (max_point.z + min_point.z) * 0.5f);
	mesh->transform([&](vertex_t::vector_t& v)
		{
			point<space::OBJECT> p = point<space::WORLD>(v.x, v.y, v.z).transform_copy(inv_mat);
			p.x -= avg_point.x;
			p.y -= avg_point.y;
			p.z -= avg_point.z;
			return u::hats2carve(p);
		});
}
void generated_static_mesh::check_material_id(scene_ctx* const scene)
{
	auto& mtl_id_attr = scene->get_mtl_id_attr();
	u32 prev_mtl_id = 0;
	bool prev_mtl_valid = false;
	for (mesh_t::face_iter i = mesh->faceBegin(); i != mesh->faceEnd(); ++i)
	{
		const mesh_t::face_t* const f = *i;
		u32 mtl_id = mtl_id_attr.getAttribute(f, 0);
		if (prev_mtl_valid)
		{
			if (mtl_id == prev_mtl_id)
			{
				m_material = mtl_id;
			}
			else
			{
				m_material = MAX_VALUE_TYPE(GLuint);
				return;
			}
		}
		prev_mtl_id = mtl_id;
		prev_mtl_valid = true;
	}
}
std::vector<std::pair<std::string, generated_mesh_param>> generated_static_mesh::get_params() const
{
	auto m = generated_mesh::get_params();
	std::vector<std::pair<std::string, generated_mesh_param>> t = {
		{ "UV0 Offset", { generated_mesh_param_type::FLOAT_4, (void*)&m_uv_offset[0].u, -MAX_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
		{ "UV1 Offset", { generated_mesh_param_type::FLOAT_4, (void*)&m_uv_offset[1].u, -MAX_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
		{ "UV2 Offset", { generated_mesh_param_type::FLOAT_4, (void*)&m_uv_offset[2].u, -MAX_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
		{ "UV3 Offset", { generated_mesh_param_type::FLOAT_4, (void*)&m_uv_offset[3].u, -MAX_PARAM_VALUE, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
	};
	m.insert(m.end(), t.begin(), t.end());
	return m;
}
const tex_coord_t* generated_static_mesh::get_uv_offset() const
{
	return m_uv_offset;
}
