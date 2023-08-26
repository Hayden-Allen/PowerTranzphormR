#pragma once
#include "pch.h"
#include "geom/carve.h"

class sgnode
{
public:
	sgnode* parent;
	std::vector<sgnode*> children;
	std::vector<point<space::OBJECT>> src_verts;
	mesh_t* mesh;
	carve::csg::CSG::OP operation;
	std::string id, name;
	bool selected, dirty;
	tmat<space::OBJECT, space::PARENT> mat;
private:
	static inline u32 s_next_id = 0;
public:
	sgnode(sgnode* p, mesh_t* m, const std::string& n, const tmat<space::OBJECT, space::PARENT>& t = tmat<space::OBJECT, space::PARENT>()) :
		parent(p),
		mesh(m),
		operation(carve::csg::CSG::OP::ALL),
		id("sgn" + std::to_string(s_next_id++)),
		name(n),
		selected(false),
		dirty(false),
		mat(t)
	{
		for (const auto& v : mesh->vertex_storage)
		{
			src_verts.emplace_back(point<space::OBJECT>(v.v.x, v.v.y, v.v.z));
		}
		set_dirty();
	}
	sgnode(carve::csg::CSG& scene, sgnode* p, carve::csg::CSG::OP op, const tmat<space::OBJECT, space::PARENT>& t = tmat<space::OBJECT, space::PARENT>()) :
		parent(p),
		mesh(nullptr),
		operation(op),
		id("sgn" + std::to_string(s_next_id++)),
		name(""),
		selected(false),
		dirty(false),
		mat(t)
	{
		set_dirty();
	}
	MGL_DCM(sgnode);
	virtual ~sgnode()
	{
		// avoid double free
		if (owns_mesh())
			delete mesh;
		for (sgnode* const child : children)
			delete child;
	}
public:
	void add_child(sgnode* const node, s64 index = -1)
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
	s64 remove_child(sgnode* const node)
	{
		const auto& it = std::find(children.begin(), children.end(), node);
		assert(it != children.end());
		const s64 index = it - children.begin();
		children.erase(it);
		// delete node;
		// not sure why this is necessary
		mesh = nullptr;
		set_dirty();

		return index;
	}
	bool is_root() const
	{
		return !parent;
	}
	bool is_leaf() const
	{
		return !children.size();
	}
	bool is_mesh() const
	{
		return operation == carve::csg::CSG::OP::ALL;
	}
	void recompute(carve::csg::CSG& scene)
	{
		dirty = false;
		if (is_leaf() && mesh)
		{
			transform_verts();
		}
		if (is_leaf())
		{
			return;
		}

		// get rid of existing mesh
		if (owns_mesh())
		{
			delete mesh;
		}
		mesh = nullptr;
		// if there are more children, add them one by one to this node's mesh
		for (u32 i = 0; i < children.size(); i++)
		{
			children[i]->recompute(scene);
			if (!children[i]->mesh)
				continue;

			if (!mesh)
			{
				mesh = children[i]->mesh;
			}
			else
			{
				mesh_t* old_mesh = mesh;
				bool delete_old_mesh = owns_mesh();
				mesh = scene.compute(mesh, children[i]->mesh, operation, nullptr, carve::csg::CSG::CLASSIFY_NORMAL);
				if (delete_old_mesh)
				{
					delete old_mesh;
				}
			}
		}
	}
	void transform(const tmat<space::OBJECT, space::OBJECT>& m)
	{
		mat *= m;
		set_dirty();
	}
	void set_transform(const tmat<space::OBJECT, space::PARENT>& new_mat)
	{
		mat = new_mat;
		set_dirty();
	}
	void set_operation(const carve::csg::CSG::OP op)
	{
		operation = op;
		set_dirty();
	}
	bool owns_mesh() const
	{
		for (sgnode* const child : children)
			if (mesh == child->mesh)
				return false;
		return true;
	}
	tmat<space::OBJECT, space::WORLD> accumulate_mats()
	{
		if (parent)
		{
			return parent->accumulate_mats().cast_copy<space::PARENT, space::WORLD>() * mat;
		}
		else
		{
			return mat.cast_copy<space::OBJECT, space::WORLD>();
		}
	}
	tmat<space::OBJECT, space::WORLD> accumulate_parent_mats()
	{
		if (parent)
		{
			return parent->accumulate_mats();
		}
		else
		{
			return tmat<space::OBJECT, space::WORLD>();
		}
	}
private:
	void set_dirty()
	{
		dirty = true;
		if (parent)
			parent->set_dirty();
	}
	void transform_verts()
	{
		size_t i = 0;
		const auto& m = accumulate_mats();
		mesh->transform([&](vertex_t::vector_t& v)
			{
				const auto& out = hats2carve(src_verts[i].transform_copy(m));
				++i;
				return out;
			});
	}
};