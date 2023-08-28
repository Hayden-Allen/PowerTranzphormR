#include "pch.h"
#include "scene_graph.h"
#include "scene_ctx.h"
#include "geom/carve.h"
#include "geom/generated_mesh.h"
#include "ui/app_ctx.h"

sgnode::sgnode(sgnode* p, generated_mesh* m, const std::string& n, const tmat<space::OBJECT, space::PARENT>& t) :
	parent(p),
	gen(m),
	operation(carve::csg::CSG::OP::ALL),
	id("sgn" + std::to_string(s_next_id++)),
	name(n),
	selected(false),
	dirty(false),
	mat(t)
{
	set_dirty();
}
sgnode::sgnode(carve::csg::CSG& scene, sgnode* p, carve::csg::CSG::OP op, const tmat<space::OBJECT, space::PARENT>& t) :
	parent(p),
	gen(nullptr),
	operation(op),
	id("sgn" + std::to_string(s_next_id++)),
	name(""),
	selected(false),
	dirty(false),
	mat(t)
{
	set_dirty();
}
sgnode::sgnode(const nlohmann::json& obj) :
	parent(nullptr),
	operation(obj["op"]),
	id(obj["id"]),
	name(obj["name"]),
	dirty(false),
	selected(false)
{
	if (!obj["gen"].is_null())
		gen = generated_mesh::create(obj["gen"]);
	else
		gen = new generated_mesh(nullptr);
	mat = json2tmat<space::OBJECT, space::PARENT>(obj["mat"]);
}
sgnode::~sgnode()
{
	// avoid double free
	if (owns_mesh())
		delete gen->mesh;
	delete gen;
	for (sgnode* const child : children)
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



s64 sgnode::get_index() const
{
	if (!parent)
		return -1;
	const auto& it = std::find(parent->children.begin(), parent->children.end(), this);
	assert(it != parent->children.end());
	return it - parent->children.begin();
}
bool sgnode::is_root() const
{
	return !parent;
}
bool sgnode::is_leaf() const
{
	return !children.size();
}
bool sgnode::is_mesh() const
{
	return operation == carve::csg::CSG::OP::ALL && gen;
}
bool sgnode::owns_mesh() const
{
	if (!gen)
		return false;
	for (const sgnode* const child : children)
		if (child->gen && gen->mesh == child->gen->mesh)
			return false;
	return true;
}
void sgnode::set_transform(const tmat<space::OBJECT, space::PARENT>& new_mat)
{
	mat = new_mat;
	set_dirty();
}
void sgnode::set_operation(const carve::csg::CSG::OP op)
{
	operation = op;
	set_dirty();
}
void sgnode::set_gen_dirty()
{
	gen->dirty = true;
	set_dirty();
}
void sgnode::set_dirty()
{
	dirty = true;
	if (parent)
		parent->set_dirty();
}
tmat<space::OBJECT, space::WORLD> sgnode::accumulate_mats() const
{
	if (parent)
		return parent->accumulate_mats().cast_copy<space::PARENT, space::WORLD>() * mat;
	return mat.cast_copy<space::OBJECT, space::WORLD>();
}
tmat<space::OBJECT, space::WORLD> sgnode::accumulate_parent_mats() const
{
	if (parent)
		return parent->accumulate_mats();
	return tmat<space::OBJECT, space::WORLD>();
}



void sgnode::add_child(sgnode* const node, const s64 index)
{
	if (index == -1)
	{
		children.push_back(node);
	}
	else
	{
		children.insert(children.begin() + index, node);
	}
	node->parent = this;
	set_dirty();
}
s64 sgnode::remove_child(sgnode* const node)
{
	const auto& it = std::find(children.begin(), children.end(), node);
	assert(it != children.end());
	const s64 index = it - children.begin();
	children.erase(it);
	// delete node;
	// not sure why this is necessary
	gen = nullptr;
	set_dirty();

	return index;
}
sgnode* sgnode::clone(app_ctx* const app, sgnode* const parent, bool create) const
{
	sgnode* ret = new sgnode();
	ret->parent = nullptr;
	for (const sgnode* const child : children)
		child->clone(app, ret, create);
	// does not clone underlying mesh_t
	if (gen)
	{
		ret->gen = gen->clone();
	}
	ret->operation = operation;
	ret->name = name;
	ret->mat = mat;
	// make sure to alert the action stack (necessary for serialization)
	if (create)
	{
		app->create_action(ret, parent);
	}
	return ret;
}
sgnode* sgnode::freeze() const
{
	sgnode* ret = new sgnode();
	ret->parent = nullptr;
	// FIXME could cause problems?
	if (gen)
	{
		ret->gen = gen->clone(accumulate_mats());
	}
	ret->operation = carve::csg::CSG::OP::ALL;
	ret->name = name;
	ret->mat = mat;
	ret->set_dirty();
	return ret;
}
void sgnode::recompute(scene_ctx* const scene)
{
	dirty = false;
	if (is_leaf())
	{
		if (gen)
		{
			if (gen->dirty)
			{
				gen->recompute(scene);
			}
			transform_verts();
		}
		return;
	}

	// get rid of existing mesh
	if (owns_mesh())
	{
		delete gen->mesh;
		delete gen;
	}
	gen = nullptr;
	// if there are more children, add them one by one to this node's mesh
	for (u32 i = 0; i < children.size(); i++)
	{
		children[i]->recompute(scene);
		if (!children[i]->gen || !children[i]->gen->mesh->vertex_storage.size())
			continue;

		if (!gen)
		{
			// raw = children[i]->get_raw();
			gen = new generated_mesh(children[i]->gen->mesh);
		}
		else
		{
			mesh_t* old_mesh = gen->mesh;
			bool delete_old_mesh = owns_mesh();
			try
			{
				gen->mesh = scene->get_csg().compute(gen->mesh, children[i]->gen->mesh, operation, nullptr, carve::csg::CSG::CLASSIFY_NORMAL);
			}
			catch (carve::exception& ex)
			{
				std::cerr << "carve::exception occurred: " << ex.what() << "\n";
			}
			if (delete_old_mesh)
			{
				delete old_mesh;
			}
		}
	}
}
void sgnode::transform(const tmat<space::OBJECT, space::OBJECT>& m)
{
	mat *= m;
	set_dirty();
}
nlohmann::json sgnode::save() const
{
	nlohmann::json obj;
	obj["parent"] = parent ? parent->id : "";

	std::vector<std::string> child_ids;
	for (const sgnode* const child : children)
		child_ids.push_back(child->id);
	obj["children"] = child_ids;

	obj["gen"] = gen->save();
	obj["op"] = operation;
	obj["id"] = id;
	obj["name"] = name;
	obj["mat"] = mat.e;

	return obj;
}



sgnode::sgnode() :
	parent(nullptr),
	gen(nullptr),
	operation(carve::csg::CSG::OP::ALL),
	id("sgn" + std::to_string(s_next_id++)),
	name(""),
	selected(false),
	dirty(false)
{}



void sgnode::transform_verts()
{
	if (is_leaf())
	{
		size_t i = 0;
		const auto& m = accumulate_mats();
		gen->mesh->transform([&](vertex_t::vector_t& v)
			{
				const auto& out = hats2carve(gen->src_verts[i].transform_copy(m));
				++i;
				return out;
			});
	}
}
