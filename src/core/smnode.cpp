#include "pch.h"
#include "smnode.h"
#include "geom/generated_mesh.h"


smnode::smnode(generated_mesh* const gen) :
	visibility_xportable(std::string("Static Mesh")),
	m_gen(gen)
{
	copy_local_verts();
	set_gen_dirty();
}
smnode::smnode(generated_mesh* const gen, const tmat<space::OBJECT, space::WORLD>& mat, const std::string& name) :
	visibility_xportable(name),
	m_mat(mat),
	m_gen(gen)
{
	const auto& inv = mat.invert_copy();
	m_gen->mesh->transform([&](vertex_t::vector_t& v)
		{
			const point<space::WORLD> p(v.x, v.y, v.z);
			return u::hats2carve(p.transform_copy(inv));
		});

	copy_local_verts();
	set_gen_dirty();
}
smnode::smnode(const nlohmann::json& obj, scene_ctx* const scene) :
	visibility_xportable(obj)
{
	m_should_snap = obj["should_snap"];
	m_snap_all = obj["snap_all"];
	m_snap_angle = obj["snap_angle"];
	if (obj["s"])
		m_gen = new generated_static_mesh(obj["m"], scene);
	else
	{
		m_gen = new generated_heightmap(obj["m"]);
		m_gen->recompute(scene);
	}
	m_mat = u::json2tmat<space::OBJECT, space::WORLD>(obj["t"]);
	copy_local_verts();
	set_gen_dirty();
}
smnode::~smnode()
{
	delete m_gen;
}



std::vector<point<space::OBJECT>>& smnode::get_local_verts()
{
	return m_local_verts;
}
const std::vector<point<space::OBJECT>>& smnode::get_local_verts() const
{
	return m_local_verts;
}
const mesh_t* smnode::get_mesh() const
{
	return m_gen->mesh;
}
std::vector<std::pair<std::string, generated_mesh_param>> smnode::get_params() const
{
	return m_gen->get_params();
}
u32 smnode::get_material() const
{
	return m_gen->get_material();
}
tmat<space::OBJECT, space::WORLD>& smnode::get_mat()
{
	return m_mat;
}
const tmat<space::OBJECT, space::WORLD>& smnode::get_mat() const
{
	return m_mat;
}
bool smnode::should_snap() const
{
	return m_should_snap;
}
bool smnode::should_snap_all() const
{
	return m_snap_all;
}
f32 smnode::get_snap_angle() const
{
	return m_snap_angle;
}
tex_coord_t smnode::get_uv_offset() const
{
	return m_gen->get_uv_offset();
}
smnode* smnode::clone(scene_ctx* const scene) const
{
	generated_mesh* cloned_gen = m_gen->clone(scene);
	cloned_gen->recompute(scene);
	smnode* result = new smnode(cloned_gen);
	result->m_mat = m_mat;
	result->m_should_snap = m_should_snap;
	result->m_snap_all = m_snap_all;
	result->m_snap_angle = m_snap_angle;
	result->set_visibility(is_visible());
	return result;
}
void smnode::set_gen_dirty()
{
	m_gen->set_dirty();
}
void smnode::set_transform(const tmat<space::OBJECT, space::WORLD>& new_mat)
{
	m_mat = new_mat;
}
void smnode::set_material(scene_ctx* const scene, const u32 mat)
{
	m_gen->set_material(scene, mat);
	set_gen_dirty();
}
void smnode::set_vertices_color(const color_t& col, scene_ctx* const scene)
{
	m_gen->set_vertices_color(col, scene);
	set_gen_dirty();
}
void smnode::center_vertices_at_origin()
{
	m_gen->center_verts_at_origin();
	copy_local_verts();
	set_gen_dirty();
}
void smnode::set_gen(generated_mesh* const gen)
{
	delete m_gen;
	m_gen = gen;
	set_gen_dirty();
	// for nonstatic, recompute will do this
	if (is_static())
		copy_local_verts();
}
void smnode::set_should_snap(const bool snap)
{
	m_should_snap = snap;
	set_gen_dirty();
}
void smnode::set_should_snap_all(const bool all)
{
	m_snap_all = all;
	set_gen_dirty();
}
void smnode::set_snap_angle(const f32 snap)
{
	m_snap_angle = snap;
	set_gen_dirty();
}
bool smnode::is_gen_dirty() const
{
	return m_gen->is_dirty();
}
bool smnode::is_static() const
{
	return m_gen->is_static();
}
void smnode::recompute(scene_ctx* const scene)
{
	m_gen->recompute(scene);
	if (m_gen->is_static())
	{
		copy_local_to_carve();
	}
	else
	{
		copy_local_verts();
	}
}
void smnode::make_static(scene_ctx* const scene)
{
	generated_static_mesh* new_gen = new generated_static_mesh(carve_clone(m_gen->mesh, scene), scene);
	delete m_gen;
	m_gen = new_gen;
}
nlohmann::json smnode::save(scene_ctx* const scene) const
{
	nlohmann::json obj = visibility_xportable::save();
	obj["t"] = m_mat.e;
	obj["m"] = m_gen->save(scene, tmat<space::WORLD, space::OBJECT>());
	obj["s"] = is_static();
	obj["should_snap"] = m_should_snap;
	obj["snap_all"] = m_snap_all;
	obj["snap_angle"] = m_snap_angle;
	return obj;
}



void smnode::copy_local_verts()
{
	m_local_verts.clear();
	for (const auto& v : m_gen->mesh->vertex_storage)
	{
		m_local_verts.emplace_back(point<space::OBJECT>(v.v.x, v.v.y, v.v.z));
	}
}
void smnode::copy_local_to_carve()
{
	size_t i = 0;
	m_gen->mesh->transform([&](vertex_t::vector_t& v)
		{
			const auto& out = u::hats2carve(m_local_verts[i]);
			++i;
			return out;
		});
}
