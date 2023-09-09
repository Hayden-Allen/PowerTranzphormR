#include "pch.h"
#include "smnode.h"
#include "geom/generated_mesh.h"

smnode::smnode(generated_mesh* const gen) :
	m_gen(gen),
	m_id(std::string("smn") + std::to_string(s_next_id++))
{
	assert(!gen->is_static());
	copy_local_verts();
}
smnode::smnode(const nlohmann::json& obj, scene_ctx* const scene) :
	m_name(obj["n"]),
	m_id(obj["id"])
{
	if (obj["s"])
		m_gen = new generated_static_mesh(obj["m"], scene);
	else
	{
		m_gen = new generated_heightmap(obj["m"]);
		m_gen->recompute(scene);
	}
	m_mat = u::json2tmat<space::OBJECT, space::WORLD>(obj["t"]);
	copy_local_verts();
	transform_verts();
}
smnode::~smnode()
{
	delete m_gen;
}



u32 smnode::get_next_id()
{
	return s_next_id;
}
void smnode::set_next_id(const u32 id)
{
	s_next_id = id;
}
void smnode::reset_next_id()
{
	s_next_id = s_first_id;
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
const std::string& smnode::get_id() const
{
	return m_id;
}
const std::string& smnode::get_name() const
{
	return m_name;
}
void smnode::set_name(const std::string& name)
{
	m_name = name;
}
void smnode::set_dirty()
{
	m_dirty = true;
}
void smnode::set_gen_dirty()
{
	assert(!is_static());
	set_dirty();
	m_gen->set_dirty();
}
void smnode::set_transform(const tmat<space::OBJECT, space::WORLD>& new_mat)
{
	m_mat = new_mat;
	set_dirty();
}
void smnode::set_material(scene_ctx* const scene, const u32 mat)
{
	m_gen->set_material(scene, mat);
	if (is_static())
	{
		set_dirty();
	}
	else
	{
		set_gen_dirty();
	}
}
void smnode::set_gen(generated_mesh* const gen)
{
	delete m_gen;
	m_gen = gen;
}
bool smnode::is_dirty() const
{
	return m_dirty;
}
bool smnode::is_static() const
{
	return m_gen->is_static();
}
void smnode::recompute(scene_ctx* const scene)
{
	assert(m_dirty);
	m_dirty = false;

	if (m_gen->is_dirty())
	{
		m_gen->recompute(scene);
		copy_local_verts();
	}

	transform_verts();
}
void smnode::make_static(scene_ctx* const scene)
{
	generated_static_mesh* new_gen = new generated_static_mesh(carve_clone(m_gen->mesh, scene), scene);
	delete m_gen;
	m_gen = new_gen;
}
nlohmann::json smnode::save(scene_ctx* const scene) const
{
	nlohmann::json obj;
	obj["id"] = m_id;
	obj["t"] = m_mat.e;
	obj["m"] = m_gen->save(scene, m_mat.invert_copy());
	obj["s"] = is_static();
	obj["n"] = m_name;
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
void smnode::transform_verts()
{
	size_t i = 0;
	assert(m_gen->mesh);
	m_gen->mesh->transform([&](vertex_t::vector_t& v)
		{
			const auto& out = u::hats2carve(m_local_verts[i].transform_copy(m_mat));
			++i;
			return out;
		});
}
