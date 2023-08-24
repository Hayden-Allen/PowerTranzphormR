#pragma once
#include "pch.h"
#include "geom/carve.h"

struct sgnode
{
public:
	sgnode* parent;
	std::vector<sgnode*> children;
	mesh_t* mesh;
	carve::csg::CSG::OP operation;
	const std::string id;
	std::string name;
	bool selected = false;
	// HATODO this should be tmat<OBJECT, PARENT>;
	// <OBJECT, WORLD> needs to be computed by traversing down from the root
	tmat<space::OBJECT, space::WORLD> mat;
public:
	// leaf node
	sgnode(sgnode* p, mesh_t* m, const tmat<space::OBJECT, space::WORLD>& t) :
		id("sgn" + std::to_string(m_next_id++)),
		name(""),
		parent(p),
		mesh(m),
		operation(carve::csg::CSG::OP::ALL),
		mat(t)
	{}
	// non-leaf node
	sgnode(carve::csg::CSG& scene, sgnode* p, carve::csg::CSG::OP op, const std::vector<sgnode*> c) :
		id("sgn" + std::to_string(m_next_id++)),
		name(""),
		parent(p),
		children(c),
		mesh(nullptr),
		operation(op)
	{
		for (sgnode* const child : children)
		{
			child->parent = this;
		}
		recompute(scene);
	}
	MGL_DCM(sgnode);
	virtual ~sgnode()
	{
		for (sgnode* child : children)
			delete child;
		delete mesh;
		mesh = nullptr;
	}
public:
	void set_operation(const carve::csg::CSG::OP op)
	{
		operation = op;
		m_dirty = true;
	}
	void add_child(sgnode* const node)
	{
		if (children.size() > 1)
		{
			delete mesh;
			mesh = nullptr;
		}
		children.push_back(node);
		node->parent = this;
		set_dirty();
	}
	void remove_child(sgnode* const node)
	{
		const auto& it = std::find(children.begin(), children.end(), node);
		if (it == children.end())
		{
			assert(false);
			return;
		}
		children.erase(it);
		delete node;
		delete mesh;
		mesh = nullptr;
		set_dirty();
	}
	bool is_root() const
	{
		return !parent;
	}
	bool is_leaf() const
	{
		return !children.size();
	}
	void set_transform(const tmat<space::OBJECT, space::WORLD>& new_mat)
	{
		transform(mat.invert_copy() * new_mat);
		mat = new_mat;
	}
	void transform(const tmat<space::OBJECT, space::OBJECT>& m)
	{
		mat *= m;
		// when we hit the bottom, mark current node as dirty and work our way back up
		if (is_leaf())
		{
			// HATODO seems to be a memory leak
			// only transform leaf meshes because the changes will get propagated back up the tree by recompute anyway
			mesh->transform([&](vertex_t::vector_t& v)
				{
					return hats2carve(point<space::OBJECT>(v.x, v.y, v.z).transform(m));
				});
			set_dirty();
		}
		// not a leaf, don't transform directly; recursively transform children instead
		else
		{
			for (sgnode* child : children)
				child->transform(m);
		}
	}
	// recomputes the scene graph from the top down
	bool recompute(carve::csg::CSG& scene)
	{
		// printf("RECOMPUTE %p\n", this);
		// if no children or hasn't been changed, stop
		if (is_leaf() || !m_dirty)
		{
			m_dirty = false;
			return false;
		}

		m_dirty = false;
		// if only one child, have nothing to recompute
		if (children.size() == 1)
		{
			// recursively recompute child
			children[0]->recompute(scene);
			// printf("RECOMPUTE %p\n", children[0]);
			mesh = children[0]->mesh;
		}
		// recompute all children recursively and merge them into one mesh
		else
		{
			delete mesh;
			children[0]->recompute(scene);
			children[1]->recompute(scene);
			// printf("RECOMPUTE %p\n", children[0]);
			// printf("RECOMPUTE %p\n", children[1]);
			mesh = scene.compute(children[0]->mesh, children[1]->mesh, operation, nullptr, carve::csg::CSG::CLASSIFY_NORMAL);
			for (s32 i = 2; i < children.size(); i++)
			{
				children[i]->recompute(scene);
				// printf("RECOMPUTE %p\n", children[i]);
				mesh_t* old_mesh = mesh;
				mesh = scene.compute(old_mesh, children[i]->mesh, operation, nullptr, carve::csg::CSG::CLASSIFY_NORMAL);
				delete old_mesh;
			}
		}
		return true;
	}
	bool is_dirty() const
	{
		return m_dirty;
	}
private:
	static inline u32 m_next_id = 1;
private:
	// recompute always needs to be called after creation
	bool m_dirty = true;
private:
	void set_dirty()
	{
		// printf("DIRTY %p\n", this);
		m_dirty = true;
		if (parent)
			parent->set_dirty();
	}
};
