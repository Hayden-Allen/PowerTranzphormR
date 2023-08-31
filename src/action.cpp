#include "pch.h"
#include "action.h"
#include "ui/app_ctx.h"
#include "scene_ctx.h"
#include "sgnode.h"

action::action(sgnode* const t) :
	target(t)
{}
action::action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes) :
	target(nodes.at(obj["t"]))
{}
action* action::create(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes)
{
	const u32 type = obj["type"];
	switch (type)
	{
	case 0: return new transform_action(obj, nodes);
	case 1: return new reparent_action(obj, nodes);
	case 2: return new create_action(obj, nodes);
	case 3: return new destroy_action(obj, nodes);
	case 4: return new freeze_action(obj, nodes);
	case 5: return new unfreeze_action(obj, nodes);
	case 6: return new rename_action(obj, nodes);
	default: assert(false);
	}
	return nullptr;
}
bool action::redo_conflict(const sgnode* const selected) const
{
	return false;
}
bool action::undo_conflict(const sgnode* const selected) const
{
	return false;
}
void action::all_nodes(std::unordered_set<const sgnode*>& nodes) const
{
	nodes.insert(target);
}



transform_action::transform_action(sgnode* const target, const tmat<space::OBJECT, space::PARENT>& old_mat, const tmat<space::OBJECT, space::PARENT>& new_mat) :
	action(target),
	initial(old_mat),
	mat(new_mat)
{}
transform_action::transform_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes) :
	action(obj, nodes),
	initial(json2tmat<space::OBJECT, space::PARENT>(obj["i"])),
	mat(json2tmat<space::OBJECT, space::PARENT>(obj["m"]))
{}
void transform_action::apply(scene_ctx* const ctx, app_ctx* const a_ctx)
{
	target->set_transform(mat);
}
void transform_action::undo(scene_ctx* const ctx, app_ctx* const a_ctx)
{
	target->set_transform(initial);
}
nlohmann::json transform_action::save() const
{
	nlohmann::json obj;
	obj["type"] = 0;
	obj["t"] = target->get_id();
	obj["i"] = initial.e;
	obj["m"] = mat.e;
	return obj;
}



reparent_action::reparent_action(sgnode* const target, sgnode* const _new_parent, const s64 _new_index) :
	action(target),
	old_parent(target->get_parent()),
	new_parent(_new_parent),
	new_index(_new_index)
{}
reparent_action::reparent_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes) :
	action(obj, nodes),
	old_parent(nodes.at(obj["op"])),
	new_parent(nodes.at(obj["np"])),
	old_index(obj["oi"]),
	new_index(obj["ni"])
{}
void reparent_action::apply(scene_ctx* const ctx, app_ctx* const a_ctx)
{
	old_index = old_parent->remove_child(target);
	new_parent->add_child(target, new_index);
}
void reparent_action::undo(scene_ctx* const ctx, app_ctx* const a_ctx)
{
	new_parent->remove_child(target);
	old_parent->add_child(target, old_index);
}
nlohmann::json reparent_action::save() const
{
	nlohmann::json obj;
	obj["type"] = 1;
	obj["t"] = target->get_id();
	obj["op"] = old_parent->get_id();
	obj["np"] = new_parent->get_id();
	obj["oi"] = old_index;
	obj["ni"] = new_index;
	return obj;
}
void reparent_action::all_nodes(std::unordered_set<const sgnode*>& nodes) const
{
	action::all_nodes(nodes);
	nodes.insert(old_parent);
	nodes.insert(new_parent);
}


create_action::create_action(sgnode* const new_node, sgnode* const _parent) :
	action(new_node),
	parent(_parent)
{}
create_action::create_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes) :
	action(obj, nodes),
	parent(nodes.at(obj["p"]))
{}
void create_action::apply(scene_ctx* const ctx, app_ctx* const a_ctx)
{
	parent->add_child(target);
}
void create_action::undo(scene_ctx* const ctx, app_ctx* const a_ctx)
{
	parent->remove_child(target);
}
bool create_action::redo_conflict(const sgnode* const selected) const
{
	return false;
}
bool create_action::undo_conflict(const sgnode* const selected) const
{
	return selected == target;
}
nlohmann::json create_action::save() const
{
	nlohmann::json obj;
	obj["type"] = 2;
	obj["t"] = target->get_id();
	obj["p"] = parent->get_id();
	return obj;
}
void create_action::all_nodes(std::unordered_set<const sgnode*>& nodes) const
{
	action::all_nodes(nodes);
	nodes.insert(parent);
}


destroy_action::destroy_action(sgnode* const target) :
	action(target),
	parent(target->get_parent()),
	index(-1)
{
	// shouldn't be able to destroy root
	assert(parent);
}
destroy_action::destroy_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes) :
	action(obj, nodes),
	parent(nodes.at(obj["p"])),
	index(obj["i"])
{}
void destroy_action::apply(scene_ctx* const ctx, app_ctx* const a_ctx)
{
	if (parent)
	{
		index = parent->remove_child(target);
	}
}
void destroy_action::undo(scene_ctx* const ctx, app_ctx* const a_ctx)
{
	if (parent)
	{
		assert(index != -1);
		parent->add_child(target, index);
	}
}
bool destroy_action::redo_conflict(const sgnode* const selected) const
{
	return selected == target;
}
bool destroy_action::undo_conflict(const sgnode* const selected) const
{
	return false;
}
nlohmann::json destroy_action::save() const
{
	nlohmann::json obj;
	obj["type"] = 3;
	obj["t"] = target->get_id();
	obj["p"] = parent->get_id();
	obj["i"] = index;
	return obj;
}
void destroy_action::all_nodes(std::unordered_set<const sgnode*>& nodes) const
{
	action::all_nodes(nodes);
	nodes.insert(parent);
}


freeze_action::freeze_action(sgnode* const target, scene_ctx* const scene) :
	action(target),
	frozen(target->freeze(scene)),
	index(target->get_index())
{
	assert(!target->is_root());
}
freeze_action::freeze_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes) :
	action(obj, nodes),
	frozen(nodes.at(obj["f"])),
	index(obj["i"])
{}
void freeze_action::apply(scene_ctx* const ctx, app_ctx* const a_ctx)
{
	sgnode* const parent = target->get_parent();
	parent->remove_child(target);
	parent->add_child(frozen, index);
	a_ctx->set_selected_sgnode(frozen);
}
void freeze_action::undo(scene_ctx* const ctx, app_ctx* const a_ctx)
{
	sgnode* const parent = frozen->get_parent();
	parent->remove_child(frozen);
	parent->add_child(target, index);
	a_ctx->set_selected_sgnode(target);
}
nlohmann::json freeze_action::save() const
{
	nlohmann::json obj;
	obj["type"] = 4;
	obj["t"] = target->get_id();
	obj["f"] = frozen->get_id();
	obj["i"] = index;
	return obj;
}
void freeze_action::all_nodes(std::unordered_set<const sgnode*>& nodes) const
{
	action::all_nodes(nodes);
	nodes.insert(frozen);
}


unfreeze_action::unfreeze_action(sgnode* const target, sgnode* const _unfrozen) :
	action(target),
	unfrozen(_unfrozen),
	index(target->get_index())
{
	assert(!target->is_root());
}
unfreeze_action::unfreeze_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes) :
	action(obj, nodes),
	unfrozen(nodes.at(obj["u"])),
	index(obj["i"])
{}
void unfreeze_action::apply(scene_ctx* const ctx, app_ctx* const a_ctx)
{
	sgnode* const parent = target->get_parent();
	parent->remove_child(target);
	parent->add_child(unfrozen, index);
	unfrozen->set_transform(target->get_mat());
	a_ctx->set_selected_sgnode(unfrozen);
}
void unfreeze_action::undo(scene_ctx* const ctx, app_ctx* const a_ctx)
{
	sgnode* const parent = unfrozen->get_parent();
	parent->remove_child(unfrozen);
	parent->add_child(target, index);
	target->set_transform(unfrozen->get_mat());
	a_ctx->set_selected_sgnode(target);
}
nlohmann::json unfreeze_action::save() const
{
	nlohmann::json obj;
	obj["type"] = 5;
	obj["t"] = target->get_id();
	obj["u"] = unfrozen->get_id();
	obj["i"] = index;
	return obj;
}
void unfreeze_action::all_nodes(std::unordered_set<const sgnode*>& nodes) const
{
	action::all_nodes(nodes);
	nodes.insert(unfrozen);
}



rename_action::rename_action(sgnode* const target, const std::string& name) :
	action(target),
	old_name(target->get_name()),
	new_name(name)
{}
rename_action::rename_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes) :
	action(obj, nodes),
	old_name(obj["o"]),
	new_name(obj["n"])
{}
void rename_action::apply(scene_ctx* const ctx, app_ctx* const a_ctx)
{
	target->set_name(new_name);
}
void rename_action::undo(scene_ctx* const ctx, app_ctx* const a_ctx)
{
	target->set_name(old_name);
}
nlohmann::json rename_action::save() const
{
	nlohmann::json obj;
	obj["type"] = 6;
	obj["t"] = target->get_id();
	obj["o"] = old_name;
	obj["n"] = new_name;
	return obj;
}
