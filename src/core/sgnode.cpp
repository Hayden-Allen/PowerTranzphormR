#include "pch.h"
#include "sgnode.h"
#include "geom/generated_mesh.h"
#include "scene_ctx.h"
#include "ui/app_ctx.h"

// nop node
sgnode::sgnode(sgnode* p, generated_mesh* m, const std::string& n, const tmat<space::OBJECT, space::PARENT>& t) :
	m_parent(p),
	m_gen(m),
	m_id(std::string("sgn") + std::to_string(s_next_id++)),
	m_name(n),
	m_mat(t)
{
	set_gen_dirty();
}
// operation node
sgnode::sgnode(sgnode* p, carve::csg::CSG::OP op, const tmat<space::OBJECT, space::PARENT>& t) :
	m_parent(p),
	m_gen(new generated_mesh(nullptr)),
	m_operation(op),
	m_id(std::string("sgn") + std::to_string(s_next_id++)),
	m_name(u::operation_to_string(op)),
	m_mat(t)
{
	assert(op != carve::csg::CSG::OP::ALL);
	set_dirty();
}
sgnode::sgnode(const nlohmann::json& obj, scene_ctx* const scene) :
	m_operation(obj["op"]),
	m_id(obj["id"]),
	m_name(obj["name"]),
	m_frozen(obj["frozen"])
{
	m_mat = u::json2tmat<space::OBJECT, space::PARENT>(obj["mat"]);

	if (!obj["gen"].is_null())
		m_gen = generated_mesh::create(obj["gen"], scene);
	else
		m_gen = new generated_mesh(nullptr);

	// if a noperation node, generated_mesh will not have an underlying mesh;
	// set it dirty so it is recomputed automatically at first call to sgnode::recompute
	if (!is_operation())
		set_gen_dirty();
	// if an operation node, gen doesn't need to be recomputed, but this node does
	else
		set_dirty();

	// if a phrozen node, gen->mesh exists (but will never be "recomputed"), so we need to grab its verts now
	if (m_frozen)
		copy_local_verts();
}
sgnode::~sgnode()
{
	delete m_gen;
	for (sgnode* const child : m_children)
		delete child;
}



u32 sgnode::get_next_id()
{
	return s_next_id;
}
void sgnode::set_next_id(const u32 id)
{
	s_next_id = id;
}
void sgnode::reset_next_id()
{
	s_next_id = s_first_id;
}



s64 sgnode::get_index() const
{
	if (!m_parent)
		return -1;
	const auto& it = std::find(m_parent->m_children.begin(), m_parent->m_children.end(), this);
	assert(it != m_parent->m_children.end());
	return it - m_parent->m_children.begin();
}
sgnode* sgnode::get_parent()
{
	return m_parent;
}
const sgnode* sgnode::get_parent() const
{
	return m_parent;
}
const std::string& sgnode::get_id() const
{
	return m_id;
}
const std::string& sgnode::get_name() const
{
	return m_name;
}
tmat<space::OBJECT, space::PARENT>& sgnode::get_mat()
{
	return m_mat;
}
const tmat<space::OBJECT, space::PARENT>& sgnode::get_mat() const
{
	return m_mat;
}
generated_mesh* sgnode::get_gen()
{
	return m_gen;
}
const generated_mesh* sgnode::get_gen() const
{
	return m_gen;
}
const std::vector<sgnode*>& sgnode::get_children() const
{
	return m_children;
}
const carve::csg::CSG::OP sgnode::get_operation() const
{
	return m_operation;
}
bool sgnode::is_root() const
{
	return !m_parent;
}
bool sgnode::is_operation() const
{
	return m_operation != carve::csg::CSG::OP::ALL;
}
bool sgnode::is_dirty() const
{
	return m_dirty;
}
bool sgnode::is_frozen() const
{
	return m_frozen;
}
void sgnode::set_transform(const tmat<space::OBJECT, space::PARENT>& new_mat)
{
	m_mat = new_mat;
	set_dirty();
}
void sgnode::transform(const tmat<space::OBJECT, space::OBJECT>& m)
{
	m_mat *= m;
	set_dirty();
}
void sgnode::set_operation(const carve::csg::CSG::OP op)
{
	// these fail when freezing a node
	/*assert(is_operation());
	assert(op != carve::csg::CSG::OP::ALL);*/
	m_operation = op;
	set_dirty();
}
void sgnode::set_dirty()
{
	set_dirty_up();
	set_dirty_down();
}
void sgnode::set_gen_dirty()
{
	m_gen->set_dirty();
	set_dirty();
}
void sgnode::set_name(const std::string& n)
{
	m_name = n;
}
tmat<space::OBJECT, space::WORLD> sgnode::accumulate_mats() const
{
	if (is_root())
		return m_mat.cast_copy<space::OBJECT, space::WORLD>();
	return accumulate_parent_mats() * m_mat;
}
tmat<space::PARENT, space::WORLD> sgnode::accumulate_parent_mats() const
{
	if (is_root())
		return tmat<space::PARENT, space::WORLD>();
	return m_parent->accumulate_mats().cast_copy<space::PARENT, space::WORLD>();
}
void sgnode::set_material(scene_ctx* const scene, u32 mtl_id)
{
	assert(!is_operation() && m_gen);
	m_gen->set_material(scene, mtl_id);
	set_dirty();
}
void sgnode::replace_material(scene_ctx* const scene, const u32 old_mtl_id, const u32 new_mtl_id)
{
	if (!is_operation() && m_gen)
	{
		m_gen->replace_material(scene, old_mtl_id, new_mtl_id);
		set_dirty();
	}
	for (const auto& child : m_children)
	{
		child->replace_material(scene, old_mtl_id, new_mtl_id);
	}
}
u32 sgnode::get_material()
{
	if (!is_operation() && m_gen)
	{
		return m_gen->get_material();
	}
	else
	{
		return 0;
	}
}
std::vector<point<space::OBJECT>>& sgnode::get_local_verts()
{
	return m_local_verts;
}



void sgnode::add_child(sgnode* const node, const s64 index)
{
	assert(index <= static_cast<s64>(m_children.size()));
	assert(!node->m_parent);
	assert(std::find(m_children.begin(), m_children.end(), node) == m_children.end());
	assert(index >= -1);

	if (index == -1)
	{
		m_children.push_back(node);
	}
	else
	{
		m_children.insert(m_children.begin() + index, node);
	}
	node->set_parent(this);

	set_dirty();
}
s64 sgnode::remove_child(sgnode* const node)
{
	assert(node->m_parent == this);
	const auto& it = std::find(m_children.begin(), m_children.end(), node);
	assert(it != m_children.end());
	node->set_parent(nullptr);
	const s64 index = it - m_children.begin();
	m_children.erase(it);
	set_dirty();
	return index;
}
sgnode* sgnode::clone(app_ctx* const app) const
{
	sgnode* ret = new sgnode(this);
	ret->m_gen = m_gen->clone(&app->scene, accumulate_mats().invert_copy());
	if (m_frozen)
		ret->copy_local_verts();
	for (const sgnode* const child : m_children)
		ret->add_child(child->clone(app));
	return ret;
}
sgnode* sgnode::clone_self_and_insert(app_ctx* const app, sgnode* const parent) const
{
	// create action for this node will be first, skip remaining n - 1
	const u32 skip_count = subtree_count() - 1;
	sgnode* ret = new sgnode(this);
	ret->m_gen = m_gen->clone(&app->scene, accumulate_mats().invert_copy());
	if (m_frozen)
		ret->copy_local_verts();
	app->create_action(ret, parent, skip_count);
	for (const sgnode* const child : m_children)
		child->clone_self_and_insert(app, ret, skip_count);
	return ret;
}
sgnode* sgnode::freeze(scene_ctx* const scene) const
{
	sgnode* ret = new sgnode(this);
	ret->m_frozen = true;
	ret->set_name("Phrozen " + get_name());
	ret->m_gen = new generated_static_mesh(m_gen->clone_mesh_to_local(scene, accumulate_mats()));
	ret->set_operation(carve::csg::CSG::OP::ALL);
	ret->copy_local_verts();
	return ret;
}
void sgnode::recompute(scene_ctx* const scene)
{
	assert(m_dirty);
	m_dirty = false;
	if (is_operation())
	{
		assert(m_local_verts.size() == 0);
		m_gen->clear();
		for (sgnode* const child : m_children)
		{
			if (child->m_dirty)
				child->recompute(scene);
			if (!child->m_gen->mesh)
				continue;
			if (!m_gen->mesh)
			{
				m_gen->copy_mesh_from(child->m_gen, scene);
			}
			else
			{
				try
				{
					mesh_t* new_mesh = scene->get_csg().compute(m_gen->mesh, child->m_gen->mesh, m_operation);
					m_gen->clear();
					m_gen->set_mesh(new_mesh);
				}
				catch (std::exception& ex)
				{
					std::cerr << "carve::exception happened: " << ex.what() << "\n";
				}
			}
		}
	}
	else
	{
		if (m_gen->is_dirty())
		{
			m_gen->recompute(scene);
			copy_local_verts();
		}
		// needs to happen for transform or when m_gen is changed
		transform_verts();
	}
}
nlohmann::json sgnode::save(scene_ctx* const scene) const
{
	nlohmann::json obj;
	// good for debugging?
	/*obj["parent"] = m_parent ? m_parent->m_id : "";
	std::vector<std::string> child_ids;
	for (const sgnode* const child : m_children)
		child_ids.push_back(child->m_id);
	obj["children"] = child_ids;*/

	obj["gen"] = m_gen->save(scene, accumulate_mats().invert_copy());
	obj["op"] = m_operation;
	obj["id"] = m_id;
	obj["name"] = m_name;
	obj["mat"] = m_mat.e;
	obj["frozen"] = m_frozen;

	return obj;
}
void sgnode::destroy(std::unordered_set<sgnode*>& freed)
{
	delete m_gen;
	for (sgnode* const child : m_children)
	{
		if (!freed.contains(child))
		{
			freed.insert(child);
			delete child;
		}
	}
	// this node will soon be deleted, don't want any double frees
	m_children.clear();
	m_gen = nullptr;
}



sgnode::sgnode(const sgnode* const original) :
	m_operation(original->m_operation),
	m_id(std::string("sgn") + std::to_string(s_next_id++)),
	m_name(original->m_name),
	m_frozen(original->m_frozen),
	m_mat(original->m_mat)
{}



void sgnode::copy_local_verts()
{
	assert(!is_operation());
	m_local_verts.clear();
	for (const auto& v : m_gen->mesh->vertex_storage)
	{
		m_local_verts.emplace_back(point<space::OBJECT>(v.v.x, v.v.y, v.v.z));
	}
}
void sgnode::transform_verts()
{
	size_t i = 0;
	const auto& m = accumulate_mats();
	assert(m_gen->mesh);
	m_gen->mesh->transform([&](vertex_t::vector_t& v)
		{
			const auto& out = u::hats2carve(m_local_verts[i].transform_copy(m));
			++i;
			return out;
		});
}
void sgnode::set_parent(sgnode* const parent)
{
	m_parent = parent;
}
void sgnode::set_dirty_up()
{
	m_dirty = true;
	if (m_parent)
		m_parent->set_dirty_up();
}
void sgnode::set_dirty_down()
{
	m_dirty = true;
	for (sgnode* const child : m_children)
		child->set_dirty_down();
}
u32 sgnode::subtree_count() const
{
	u32 count = 1;
	for (const sgnode* const child : m_children)
		count += child->subtree_count();
	return count;
}
void sgnode::clone_self_and_insert(app_ctx* const app, sgnode* const parent, const u32 count) const
{
	sgnode* ret = new sgnode(this);
	ret->m_gen = m_gen->clone(&app->scene);
	if (m_frozen)
		ret->copy_local_verts();
	app->create_action(ret, parent, count);
	for (const sgnode* const child : m_children)
		child->clone_self_and_insert(app, ret, count);
}
