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
	// recomputes the scene graph from this node up
	void recompute(carve::csg::CSG& scene)
	{
		// leaf nodes do not have children and so do not perform any CSG operations
		if (is_leaf()) return;

		delete mesh;
		// children do not need to be recomputed, because any change to this mesh does not affect them
		mesh = scene.compute(left->mesh, right->mesh, operation, nullptr, carve::csg::CSG::CLASSIFY_EDGE);
		// parent needs to be recomputed now that this node has changed
		if (parent)
			parent->recompute(scene);
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