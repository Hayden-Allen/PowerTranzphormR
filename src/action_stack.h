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
	MGL_DCM(action);
	virtual ~action() {}
public:
	virtual void apply(scene_ctx* const ctx) = 0;
	virtual void undo(scene_ctx* const ctx) = 0;
	virtual bool redo_conflict(const sgnode* const selected) const
	{
		return false;
	}
	virtual bool undo_conflict(const sgnode* const selected) const
	{
		return false;
	}
};
struct transform_action : public action
{
public:
	tmat<space::OBJECT, space::PARENT> initial;
	tmat<space::OBJECT, space::PARENT> mat;
public:
	transform_action(sgnode* const target, const tmat<space::OBJECT, space::PARENT>& old_mat, const tmat<space::OBJECT, space::PARENT>& new_mat) :
		action(target),
		initial(old_mat),
		mat(new_mat)
	{}
	MGL_DCM(transform_action);
public:
	void apply(scene_ctx* const ctx) override
	{
		target->set_transform(mat);
	}
	void undo(scene_ctx* const ctx) override
	{
		target->set_transform(initial);
	}
};
struct reparent_action : public action
{
public:
	sgnode *old_parent, *new_parent;
public:
	reparent_action(sgnode* const target, sgnode* const _old_parent, sgnode* const _new_parent) :
		action(target),
		old_parent(_old_parent),
		new_parent(_new_parent)
	{}
	MGL_DCM(reparent_action);
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
	create_action(sgnode* const new_node, sgnode* const _parent) :
		action(new_node),
		parent(_parent)
	{}
	MGL_DCM(create_action);
public:
	void apply(scene_ctx* const ctx) override
	{
		parent->add_child(target);
	}
	void undo(scene_ctx* const ctx) override
	{
		parent->remove_child(target);
	}
	bool redo_conflict(const sgnode* const selected) const
	{
		return false;
	}
	bool undo_conflict(const sgnode* const selected) const
	{
		return selected == target;
	}
};
struct destroy_action : public action
{
public:
	sgnode* parent;
	s64 index;
public:
	destroy_action(sgnode* const target) :
		action(target),
		parent(target->parent),
		index(-1)
	{
		// shouldn't be able to destroy root
		assert(parent);
	}
	MGL_DCM(destroy_action);
public:
	void apply(scene_ctx* const ctx) override
	{
		if (parent)
		{
			index = parent->remove_child(target);
		}
	}
	void undo(scene_ctx* const ctx) override
	{
		if (parent)
		{
			assert(index != -1);
			parent->add_child(target, index);
		}
	}
	bool redo_conflict(const sgnode* const selected) const
	{
		return selected == target;
	}
	bool undo_conflict(const sgnode* const selected) const
	{
		return false;
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
	void transform(sgnode* const t, const tmat<space::OBJECT, space::PARENT>& old_mat, const tmat<space::OBJECT, space::PARENT>& new_mat)
	{
		new_action(new transform_action(t, old_mat, new_mat), true);
	}
	void reparent(sgnode* const target, sgnode* const old_parent, sgnode* const new_parent)
	{
		new_action(new reparent_action(target, old_parent, new_parent), true);
	}
	void create(sgnode* const target, sgnode* const parent)
	{
		new_action(new create_action(target, parent), true);
	}
	void destroy(sgnode* const target)
	{
		new_action(new destroy_action(target), true);
	}
	bool can_undo()
	{
		return m_past.size() > 0;
	}
	bool can_redo()
	{
		return m_future.size() > 0;
	}
	// undo last action made and move it to the redo stack
	action* undo()
	{
		if (can_undo())
		{
			action* a = m_past.back();
			m_past.pop_back();
			a->undo(m_ctx);
			m_future.push_back(a);
			return a;
		}
		return nullptr;
	}
	// redo last action undone and move it to the undo stack
	action* redo()
	{
		if (can_redo())
		{
			action* a = m_future.back();
			m_future.pop_back();
			a->apply(m_ctx);
			m_past.push_back(a);
			return a;
		}
		return nullptr;
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