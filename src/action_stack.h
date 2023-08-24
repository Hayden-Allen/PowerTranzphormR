#pragma once
#include "pch.h"
#include "scene_ctx.h"
#include "scene_graph.h"

struct action
{
public:
	sgnode* target;
public:
	action(sgnode* const t) :
		target(t)
	{}
	virtual ~action() {}
public:
	virtual void apply(scene_ctx* const ctx) = 0;
	virtual void undo(scene_ctx* const ctx) = 0;
};
struct transform_action : public action
{
public:
	tmat<space::OBJECT, space::OBJECT> mat;
public:
	transform_action(sgnode* const t, const tmat<space::OBJECT, space::OBJECT>& m) :
		action(t),
		mat(m)
	{}
public:
	void apply(scene_ctx* const ctx) override
	{
		target->transform(mat);
	}
	void undo(scene_ctx* const ctx) override
	{
		target->transform(mat.invert_copy());
	}
};
struct reparent_action : public action
{
public:
	sgnode *old_parent, *new_parent;
public:
	reparent_action(sgnode* const t, sgnode* const o, sgnode* const n) :
		action(t),
		old_parent(o),
		new_parent(n)
	{}
public:
	void apply(scene_ctx* const ctx) override
	{
		old_parent->remove_child(target);
		new_parent->add_child(target);
	}
	void undo(scene_ctx* const ctx) override
	{
		new_parent->remove_child(target);
		old_parent->add_child(target);
	}
};
struct create_action : public action
{
public:
	sgnode* parent;
public:
	create_action(sgnode* const t, sgnode* const p) :
		action(t),
		parent(p)
	{}
public:
	void apply(scene_ctx* const ctx) override
	{
		parent->add_child(target);
	}
	void undo(scene_ctx* const ctx) override
	{
		parent->remove_child(target);
	}
};
struct destroy_action : public action
{
public:
	sgnode* parent;
public:
	destroy_action(sgnode* const t, sgnode* const p) :
		action(t),
		parent(p)
	{}
public:
	void apply(scene_ctx* const ctx) override
	{
		parent->remove_child(target);
	}
	void undo(scene_ctx* const ctx) override
	{
		parent->add_child(target);
	}
};

class action_stack
{
public:
	action_stack(scene_ctx* const sc) :
		m_ctx(sc)
	{}
	MGL_DCM(action_stack);
	virtual ~action_stack()
	{
		// these are all heap-allocated for polymorphism, delete them
		for (action* a : m_past)
			delete a;
		for (action* a : m_future)
			delete a;
	}
public:
	void transform(sgnode* const target, const tmat<space::OBJECT, space::OBJECT>& m, const bool apply)
	{
		new_action(new transform_action(target, m), apply);
	}
	void reparent(sgnode* const target, sgnode* const old_parent, sgnode* const new_parent)
	{
		new_action(new reparent_action(target, old_parent, new_parent), true);
	}
	void create(sgnode* const target, sgnode* const parent)
	{
		new_action(new create_action(target, parent), true);
	}
	void destroy(sgnode* const target, sgnode* const parent)
	{
		new_action(new destroy_action(target, parent), true);
	}
	// undo last action made and move it to the redo stack
	void undo()
	{
		if (m_past.size())
		{
			action* a = m_past.back();
			m_past.pop_back();
			a->undo(m_ctx);
			m_future.push_back(a);
		}
	}
	// redo last action undone and move it to the undo stack
	void redo()
	{
		if (m_future.size())
		{
			action* a = m_future.back();
			m_future.pop_back();
			a->apply(m_ctx);
			m_past.push_back(a);
		}
	}
private:
	scene_ctx* m_ctx;
	// ctrl-z (undo)
	std::vector<action*> m_past;
	// ctrl-y (redo)
	std::vector<action*> m_future;
private:
	// create and apply a new action
	void new_action(action* const a, const bool apply)
	{
		if (apply)
			a->apply(m_ctx);
		m_past.push_back(a);
		// new action has been made, previous future no longer exists
		for (action* f : m_future)
			delete f;
		m_future.clear();
	}
};