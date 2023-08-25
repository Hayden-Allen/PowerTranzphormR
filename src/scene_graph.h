#pragma once
#include "pch.h"
#include "geom/carve.h"

class sgnode
{
public:
	sgnode* parent;
	std::vector<sgnode*> children;
	mesh_t* mesh;
	carve::csg::CSG::OP operation;
	std::string id, name;
	bool selected, dirty;
	// HATODO should be OBJECT=>PARENT, where PARENT==WORLD @ root
	tmat<space::OBJECT, space::WORLD> mat;
private:
	static inline u32 s_next_id = 0;
public:
	sgnode(sgnode* p, mesh_t* m, const std::string& n, const tmat<space::OBJECT, space::WORLD>& t) :
		parent(p),
		mesh(m),
		operation(carve::csg::CSG::OP::ALL),
		id("sgn" + std::to_string(s_next_id++)),
		name(n),
		selected(false),
		dirty(false),
		mat(t)
	{}
	sgnode(carve::csg::CSG& scene, sgnode* p, carve::csg::CSG::OP op, const std::vector<sgnode*>& c) :
		parent(p),
		children(c),
		mesh(nullptr),
		operation(op),
		id("sgn" + std::to_string(s_next_id++)),
		name(""),
		selected(false),
		dirty(false)
	{
		for (sgnode* const child : children)
			child->parent = this;
		recompute(scene);
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
		children.erase(it);
		// delete node;
		// not sure why this is necessary
		mesh = nullptr;
		set_dirty();

		return it - children.begin();
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
		if (is_leaf() && mesh)
		{
			mesh->transform([&](vertex_t::vector_t& v)
				{
					return hats2carve(point<space::OBJECT>(v.x, v.y, v.z).transform(m));
				});
			set_dirty();
		}
		else
		{
			for (sgnode* const child : children)
				child->transform(m);
		}
	}
	void set_transform(const tmat<space::OBJECT, space::WORLD>& new_mat)
	{
		const auto& inv_mat = mat.invert_copy().cast_copy<space::OBJECT, space::OBJECT>();
		mat = new_mat;
		const auto& casted_mat = mat.cast_copy<space::OBJECT, space::OBJECT>();
		if (is_leaf() && mesh)
		{
			// FIXME: Scaling to zero makes it impossible to scale back up to any other value
			// We need to enforce a minimum scale of some epsilon value
			mesh->transform([&](vertex_t::vector_t& v)
				{
					return hats2carve(point<space::OBJECT>(v.x, v.y, v.z).transform(inv_mat));
				});
			mesh->transform([&](vertex_t::vector_t& v)
				{
					return hats2carve(point<space::OBJECT>(v.x, v.y, v.z).transform(casted_mat));
				});
			set_dirty();
		}
		else
		{
			for (sgnode* const child : children)
				child->set_transform(mat);
		}
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
private:
	void set_dirty()
	{
		dirty = true;
		if (parent)
			parent->set_dirty();
	}
};