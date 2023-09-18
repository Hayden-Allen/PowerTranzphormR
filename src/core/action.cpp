#include "pch.h"
#include "action.h"
#include "ui/app_ctx.h"
#include "scene_ctx.h"
#include "sgnode.h"

action::action(sgnode* const t, const u32 sc) :
	target(t),
	skip_count(sc)
{}
action::action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes) :
	target(nodes.at(obj["t"])),
	skip_count(obj["sc"])
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
nlohmann::json action::save() const
{
	nlohmann::json obj;
	obj["sc"] = skip_count;
	return obj;
}



transform_action::transform_action(sgnode* const target, const tmat<space::OBJECT, space::PARENT>& old_mat, const tmat<space::OBJECT, space::PARENT>& new_mat) :
	action(target),
	initial(old_mat),
	mat(new_mat)
{}
transform_action::transform_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes) :
	action(obj, nodes),
	initial(u::json2tmat<space::OBJECT, space::PARENT>(obj["i"])),
	mat(u::json2tmat<space::OBJECT, space::PARENT>(obj["m"]))
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
	nlohmann::json obj = action::save();
	obj["type"] = 0;
	obj["t"] = target->get_id();
	obj["i"] = initial.e;
	obj["m"] = mat.e;
	return obj;
}



reparent_action::reparent_action(sgnode* const target, sgnode* const _new_parent, const s64 _new_index, const u32 _skip_count) :
	action(target, _skip_count),
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
	nlohmann::json obj = action::save();
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



create_action::create_action(sgnode* const new_node, sgnode* const _parent, const u32 skip_count) :
	action(new_node, skip_count),
	parent(_parent)
{}
create_action::create_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes) :
	action(obj, nodes),
	parent(nullptr)
{
	if (obj.contains("p"))
		parent = nodes.at(obj["p"]);
}
void create_action::apply(scene_ctx* const ctx, app_ctx* const a_ctx)
{
	if (parent)
		parent->add_child(target);
}
void create_action::undo(scene_ctx* const ctx, app_ctx* const a_ctx)
{
	if (parent)
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
	nlohmann::json obj = action::save();
	obj["type"] = 2;
	obj["t"] = target->get_id();
	if (parent)
		obj["p"] = parent->get_id();
	return obj;
}
void create_action::all_nodes(std::unordered_set<const sgnode*>& nodes) const
{
	action::all_nodes(nodes);
	if (parent)
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
	nlohmann::json obj = action::save();
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
	nlohmann::json obj = action::save();
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
	nlohmann::json obj = action::save();
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
	nlohmann::json obj = action::save();
	obj["type"] = 6;
	obj["t"] = target->get_id();
	obj["o"] = old_name;
	obj["n"] = new_name;
	return obj;
}



set_op_action::set_op_action(sgnode* const target, const op_t newo) :
	action(target),
	old_op(target->get_operation()),
	new_op(newo)
{
	assert(target->is_operation());
	assert(old_op == op_t::A_MINUS_B || old_op == op_t::INTERSECTION || old_op == op_t::UNION);
	assert(new_op == op_t::A_MINUS_B || new_op == op_t::INTERSECTION || new_op == op_t::UNION);
}
set_op_action::set_op_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes) :
	action(obj, nodes),
	old_op(obj["o"]),
	new_op(obj["n"])
{}
void set_op_action::apply(scene_ctx* const ctx, app_ctx* const a_ctx)
{
	const std::string display_name = target->get_name();
	const std::string& op_name = u::operation_to_string(target->get_operation());
	if (display_name == op_name)
		target->set_name(u::operation_to_string(new_op));
	target->set_operation(new_op);
}
void set_op_action::undo(scene_ctx* const ctx, app_ctx* const a_ctx)
{
	const std::string display_name = target->get_name();
	const std::string& op_name = u::operation_to_string(target->get_operation());
	if (display_name == op_name)
		target->set_name(u::operation_to_string(old_op));
	target->set_operation(old_op);
}
nlohmann::json set_op_action::save() const
{
	nlohmann::json obj = action::save();
	obj["o"] = old_op;
	obj["n"] = new_op;
	return obj;
}
