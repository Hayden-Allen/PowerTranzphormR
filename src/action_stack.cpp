#include "pch.h"
#include "action_stack.h"


static void all_nodes(const sgnode* const cur, std::unordered_set<const sgnode*>& nodes)
{
	nodes.insert(cur);
	for (const sgnode* const child : cur->children)
		all_nodes(child, nodes);
}


action_stack::action_stack(scene_ctx* const sc) :
	m_ctx(sc)
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
void action_stack::create(sgnode* const target, sgnode* const parent)
{
	new_action(new create_action(target, parent), true);
}
void action_stack::destroy(sgnode* const target)
{
	new_action(new destroy_action(target), true);
}
sgnode* action_stack::freeze(sgnode* const target)
{
	freeze_action* const a = new freeze_action(target);
	new_action(a, true);
	return a->frozen;
}
void action_stack::unfreeze(sgnode* const target, sgnode* const unfrozen)
{
	new_action(new unfreeze_action(target, unfrozen), true);
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
		action* a = m_past.back();
		m_past.pop_back();
		a->undo(m_ctx);
		m_future.push_back(a);
		return a;
	}
	return nullptr;
}
action* action_stack::redo()
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
void action_stack::save(std::ofstream& out, const sgnode* const root) const
{
	// combine past and future events into one big list for simplicity
	std::vector<action*> all;
	all.reserve(m_past.size() + m_future.size());
	all.insert(all.end(), m_past.begin(), m_past.end());
	all.insert(all.end(), m_future.begin(), m_future.end());

	// write out all nodes
	std::unordered_set<const sgnode*> nodes;
	nodes.insert(root);
	for (action* const a : all)
		all_nodes(a->target, nodes);
	out << nodes.size() << "\n";
	for (const sgnode* const node : nodes)
	{
		out << std::string(node->save().dump()) << "\n";
	}

	// write out all events
	out << m_past.size() << " " << m_future.size() << "\n";
	for (const action* const a : all)
		out << std::string(a->save().dump()) << "\n";

	// write out next unique sgnode id
	out << sgnode::get_next_id() << "\n";
}
sgnode* action_stack::load(std::ifstream& in)
{
	clear();

	std::string line;

	// read in all nodes
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
	// read in (and apply) past events
	m_past.reserve(past_count);
	for (u64 i = 0; i < past_count; i++)
	{
		std::getline(in, line);
		const nlohmann::json obj = nlohmann::json::parse(line);
		action* const a = action::create(obj, nodes);
		m_past.push_back(a);
		a->apply(m_ctx);
	}
	// read in future events
	m_future.reserve(future_count);
	for (u64 i = 0; i < future_count; i++)
	{
		std::getline(in, line);
		const nlohmann::json obj = nlohmann::json::parse(line);
		action* const a = action::create(obj, nodes);
		m_future.push_back(a);
	}

	// need to know what unique id to start making new sgnodes at
	u32 next_id = 0;
	in >> next_id;
	sgnode::set_next_id(next_id);

	// return the parent
	for (const auto& pair : nodes)
		if (!pair.second->parent)
			return pair.second;
	// no parent found, something broke
	assert(false);
	return nullptr;
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



void action_stack::new_action(action* const a, const bool apply)
{
	if (apply)
		a->apply(m_ctx);
	m_past.push_back(a);
	// new action has been made, previous future no longer exists
	for (action* f : m_future)
		delete f;
	m_future.clear();
}
void action_stack::clear(sgnode* const node, std::unordered_set<sgnode*> freed)
{
	for (sgnode* const child : node->children)
		clear(child, freed);
	if (!freed.contains(node))
	{
		delete node;
		freed.insert(node);
	}
}
