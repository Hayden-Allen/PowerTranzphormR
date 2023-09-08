#include "pch.h"
#include "smnode.h"
#include "geom/generated_mesh.h"

smnode::smnode(generated_mesh* const gen) :
	m_gen(gen)
{
	copy_local_verts();
}
smnode::~smnode()
{
	delete m_gen;
}



void smnode::set_transform(const tmat<space::OBJECT, space::WORLD>& new_mat)
{
	m_mat = new_mat;
	set_dirty();
}
void smnode::set_dirty()
{
	m_dirty = true;
}
void smnode::set_gen_dirty()
{
	set_dirty();
	m_gen->set_dirty();
}
bool smnode::is_dirty() const
{
	return m_dirty;
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
const std::vector<point<space::OBJECT>>& smnode::get_local_verts() const
{
	return m_local_verts;
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
