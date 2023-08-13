#pragma once
#include "pch.h"
#include "carve.h"

struct sgnode
{
	friend class scene_graph;
public:
	sgnode *parent, *left, *right;
	mesh_t* mesh;
	carve::csg::CSG::OP operation;
public:
	// leaf node
	sgnode(sgnode* p, mesh_t* m) :
		parent(p),
		left(nullptr),
		right(nullptr),
		mesh(m),
		operation(carve::csg::CSG::OP::ALL)
	{}
	// non-leaf node
	sgnode(carve::csg::CSG& scene, sgnode* p, carve::csg::CSG::OP op, sgnode* l, sgnode* r) :
		parent(p),
		left(l),
		right(r),
		mesh(nullptr),
		operation(op)
	{
		recompute(scene);
	}
	MGL_DCM(sgnode);
	virtual ~sgnode()
	{
		// printf("DELETE %p\n", mesh);
		delete mesh;
	}
public:
	bool is_root() const
	{
		return !parent;
	}
	bool is_leaf() const
	{
		return !left && !right;
	}
	void transform(carve::csg::CSG& scene, const carve::math::Matrix& m)
	{
		transform_base(scene, m);
		recompute(scene);
	}
	// recomputes the scene graph from this node up
	void recompute(carve::csg::CSG& scene)
	{
		if (is_leaf())
		{
			if (parent)
				parent->recompute(scene);
			return;
		}

		delete mesh;
		// children do not need to be recomputed, because any change to this mesh does not affect them
		mesh = scene.compute(left->mesh, right->mesh, operation, nullptr, carve::csg::CSG::CLASSIFY_NORMAL);
		// parent needs to be recomputed now that this node has changed
		if (parent)
			parent->recompute(scene);
	}
private:
	void transform_base(carve::csg::CSG& scene, const carve::math::Matrix& m)
	{
		if (!is_leaf())
		{
			left->transform_base(scene, m);
			right->transform_base(scene, m);
		}
		mesh->transform([&](vertex_t::vector_t& v)
			{
				return m * v;
			});
	}
};

class scene_graph
{
public:
	scene_graph()
	{
		std::vector<face_t*> faces;
		m_root = new sgnode(nullptr, new mesh_t(faces));
	}
	MGL_DCM(scene_graph);
	virtual ~scene_graph()
	{
		delete m_root;
	}
private:
	sgnode* m_root;
	carve::csg::CSG m_scene;
};