#include "pch.h"
#include "action_stack.h"
#include "action.h"
#include "ui/app_ctx.h"
#include "scene_ctx.h"
#include "sgnode.h"


static void all_nodes(const sgnode* const cur, std::unordered_set<const sgnode*>& nodes)
{
	nodes.insert(cur);
	for (const sgnode* const child : cur->get_children())
		all_nodes(child, nodes);
}



action_stack::action_stack(scene_ctx* const sc, app_ctx* const ac) :
	m_ctx(sc),
	m_app_ctx(ac)
{}
action_stack::~action_stack()
{
	// these are all heap-allocated for polymorphism, delete them
	for (action* a : m_past)
		delete a;
	for (action* a : m_future)
		delete a;
}



void action_stack::transform(sgnode* const t, const tmat<space::OBJECT, space::PARENT>& old_mat, const tmat<space::OBJECT, space::PARENT>& new_mat)
{
	new_action(new transform_action(t, old_mat, new_mat), true);
}
void action_stack::reparent(sgnode* const target, sgnode* const new_parent, const s64 new_index)
{
	new_action(new reparent_action(target, new_parent, new_index), true);
}
void action_stack::create(sgnode* const target, sgnode* const parent, const u32 skip_count)
{
	new_action(new create_action(target, parent, skip_count), true);
}
void action_stack::destroy(sgnode* const target)
{
	new_action(new destroy_action(target), true);
}
sgnode* action_stack::freeze(sgnode* const target)
{
	freeze_action* const a = new freeze_action(target, m_ctx);
	new_action(a, true);
	return a->frozen;
}
void action_stack::unfreeze(sgnode* const target, sgnode* const unfrozen)
{
	new_action(new unfreeze_action(target, unfrozen), true);
}
void action_stack::rename(sgnode* const target, const std::string& new_name)
{
	new_action(new rename_action(target, new_name), true);
}
bool action_stack::can_undo()
{
	return m_past.size() > 0;
}
bool action_stack::can_redo()
{
	return m_future.size() > 0;
}
action* action_stack::undo()
{
	if (can_undo())
	{
		// undo one action
		action* a = m_past.back();
		m_past.pop_back();
		a->undo(m_ctx, m_app_ctx);
		m_future.push_back(a);

		// if this action says to skip over following actions, also process them
		const u32 skip_count = a->skip_count;
		for (u32 i = 0; i < skip_count; i++)
		{
			action* a2 = m_past.back();
			assert(a2->skip_count == skip_count);
			m_past.pop_back();
			a2->undo(m_ctx, m_app_ctx);
			m_future.push_back(a2);
		}

		m_modified = true;
		return a;
	}
	return nullptr;
}
action* action_stack::redo()
{
	if (can_redo())
	{
		// redo one action
		action* a = m_future.back();
		m_future.pop_back();
		a->apply(m_ctx, m_app_ctx);
		m_past.push_back(a);

		// if this action says to skip over following actions, also process them
		const u32 skip_count = a->skip_count;
		for (u32 i = 0; i < skip_count; i++)
		{
			action* a2 = m_future.back();
			assert(a2->skip_count == skip_count);
			m_future.pop_back();
			a2->apply(m_ctx, m_app_ctx);
			m_past.push_back(a2);
		}

		m_modified = true;
		return a;
	}
	return nullptr;
}
void action_stack::save(std::ofstream& out, const sgnode* const root) const
{
	// combine past and future events into one big list for simplicity
	std::vector<action*> all;
	all.reserve(m_past.size() + m_future.size());
	all.insert(all.end(), m_past.begin(), m_past.end());
	all.insert(all.end(), m_future.begin(), m_future.end());

	// find all nodes
	std::unordered_set<const sgnode*> nodes;
	nodes.insert(root);
	if (m_app_ctx->clipboard)
	{
		nodes.insert(m_app_ctx->clipboard);
	}
	for (action* const a : all)
		a->all_nodes(nodes);

	// meta object
	nlohmann::json meta;
	meta["nn"] = nodes.size();
	meta["np"] = m_past.size();
	meta["nf"] = m_future.size();
	meta["ni"] = sgnode::get_next_id();
	out << meta << "\n";

	// write out all nodes
	for (const sgnode* const node : nodes)
	{
		out << node->save(&m_app_ctx->scene) << "\n";
	}

	// write out all events
	for (const action* const a : all)
		out << a->save() << "\n";

	m_modified = false;
}
std::unordered_map<std::string, sgnode*> action_stack::load(std::ifstream& in)
{
	clear();

	std::string line;

	// read in meta block
	const nlohmann::json& meta = u::next_line_json(in);

	// read in all nodes
	std::unordered_map<std::string, sgnode*> nodes;
	nodes.reserve(meta["nn"]);
	for (u64 i = 0; i < meta["nn"]; i++)
	{
		std::getline(in, line);
		const nlohmann::json obj = nlohmann::json::parse(line);
		sgnode* node = new sgnode(obj, &m_app_ctx->scene);
		node->set_dirty();
		nodes[obj["id"]] = node;
	}

	// read in (and apply) past events
	m_past.reserve(meta["np"]);
	for (u64 i = 0; i < meta["np"]; i++)
	{
		std::getline(in, line);
		const nlohmann::json obj = nlohmann::json::parse(line);
		action* const a = action::create(obj, nodes);
		m_past.push_back(a);
		a->apply(m_ctx, m_app_ctx);
	}
	// read in future events
	m_future.reserve(meta["nf"]);
	for (u64 i = 0; i < meta["nf"]; i++)
	{
		std::getline(in, line);
		const nlohmann::json obj = nlohmann::json::parse(line);
		action* const a = action::create(obj, nodes);
		m_future.push_back(a);
	}

	// need to know what unique id to start making new sgnodes at
	sgnode::set_next_id(meta["ni"]);

	return nodes;
}
void action_stack::clear()
{
	std::unordered_set<sgnode*> freed;
	for (action* const a : m_past)
	{
		clear(a->target, freed);
	}
	for (action* const a : m_future)
	{
		clear(a->target, freed);
	}
	m_past.clear();
	m_future.clear();
}
bool action_stack::get_modified() const
{
	return m_modified;
}



void action_stack::new_action(action* const a, const bool apply)
{
	if (apply)
		a->apply(m_ctx, m_app_ctx);
	m_past.push_back(a);
	// new action has been made, previous future no longer exists
	for (action* f : m_future)
		delete f;
	m_future.clear();
	m_modified = true;
}
void action_stack::clear(sgnode* const node, std::unordered_set<sgnode*>& freed)
{
	for (sgnode* const child : node->get_children())
		clear(child, freed);
	if (!freed.contains(node))
	{
		freed.insert(node);
		node->destroy(freed);
		delete node;
	}
	m_modified = false;
}
