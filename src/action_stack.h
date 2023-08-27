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
	action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes) :
		target(nodes.at(obj["t"]))
	{}
	MGL_DCM(action);
	virtual ~action() {}
public:
	static action* create(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes);
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
	virtual nlohmann::json save() const = 0;
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
	transform_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes) :
		action(obj, nodes),
		initial(json2tmat<space::OBJECT, space::PARENT>(obj["i"])),
		mat(json2tmat<space::OBJECT, space::PARENT>(obj["m"]))
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
	nlohmann::json save() const override
	{
		nlohmann::json obj;
		obj["type"] = 0;
		obj["t"] = target->id;
		obj["i"] = initial.e;
		obj["m"] = mat.e;
		return obj;
	}
};
struct reparent_action : public action
{
public:
	sgnode *old_parent, *new_parent;
	s64 old_index = -1, new_index = -1;
public:
	reparent_action(sgnode* const target, sgnode* const _new_parent, const s64 _new_index) :
		action(target),
		old_parent(target->parent),
		new_parent(_new_parent),
		new_index(_new_index)
	{}
	reparent_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes) :
		action(obj, nodes),
		old_parent(nodes.at(obj["op"])),
		new_parent(nodes.at(obj["np"])),
		old_index(obj["oi"]),
		new_index(obj["ni"])
	{}
	MGL_DCM(reparent_action);
public:
	void apply(scene_ctx* const ctx) override
	{
		old_index = old_parent->remove_child(target);
		new_parent->add_child(target, new_index);
	}
	void undo(scene_ctx* const ctx) override
	{
		new_parent->remove_child(target);
		old_parent->add_child(target, old_index);
	}
	nlohmann::json save() const override
	{
		nlohmann::json obj;
		obj["type"] = 1;
		obj["t"] = target->id;
		obj["op"] = old_parent->id;
		obj["np"] = new_parent->id;
		obj["oi"] = old_index;
		obj["ni"] = new_index;
		return obj;
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
	create_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes) :
		action(obj, nodes),
		parent(nodes.at(obj["p"]))
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
	nlohmann::json save() const override
	{
		nlohmann::json obj;
		obj["type"] = 2;
		obj["t"] = target->id;
		obj["p"] = parent->id;
		return obj;
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
	destroy_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes) :
		action(obj, nodes),
		parent(nodes.at(obj["p"])),
		index(obj["i"])
	{}
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
	nlohmann::json save() const override
	{
		nlohmann::json obj;
		obj["type"] = 3;
		obj["t"] = target->id;
		obj["p"] = parent->id;
		obj["i"] = index;
		return obj;
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
	void reparent(sgnode* const target, sgnode* const new_parent, const s64 new_index)
	{
		new_action(new reparent_action(target, new_parent, new_index), true);
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
	void save(std::ofstream& out, const sgnode* const root) const
	{
		std::vector<action*> all;
		all.reserve(m_past.size() + m_future.size());
		all.insert(all.end(), m_past.begin(), m_past.end());
		all.insert(all.end(), m_future.begin(), m_future.end());

		std::unordered_set<const sgnode*> nodes;
		nodes.insert(root);
		for (action* const a : all)
			nodes.insert(a->target);

		out << nodes.size() << "\n";
		for (const sgnode* const node : nodes)
		{
			// node->save(out);
			out << std::string(node->save().dump()) << "\n";
		}

		out << m_past.size() << " " << m_future.size() << "\n";
		for (const action* const a : all)
			out << std::string(a->save().dump()) << "\n";

		out << sgnode::get_next_id() << "\n";
	}
	sgnode* load(std::ifstream& in)
	{
		std::string line;

		u64 num_nodes;
		in >> num_nodes;
		std::getline(in, line);
		std::unordered_map<std::string, sgnode*> nodes;
		nodes.reserve(num_nodes);
		for (u64 i = 0; i < num_nodes; i++)
		{
			std::getline(in, line);
			const nlohmann::json obj = nlohmann::json::parse(line);
			sgnode* node = new sgnode(obj);
			node->set_dirty();
			nodes[obj["id"]] = node;
		}

		u64 past_count, future_count;
		in >> past_count >> future_count;
		std::getline(in, line);
		m_past.reserve(past_count);
		m_future.reserve(future_count);
		for (u64 i = 0; i < past_count; i++)
		{
			std::getline(in, line);
			const nlohmann::json obj = nlohmann::json::parse(line);
			action* const a = action::create(obj, nodes);
			m_past.push_back(a);
			a->apply(m_ctx);
		}

		u32 next_id = 0;
		in >> next_id;
		sgnode::set_next_id(next_id);

		for (const auto& pair : nodes)
			if (!pair.second->parent)
				return pair.second;
		assert(false);
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