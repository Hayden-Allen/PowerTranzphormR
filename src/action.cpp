#include "pch.h"
#include "action.h"

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
void transform_action::apply(scene_ctx* const ctx)
{
	target->set_transform(mat);
}
void transform_action::undo(scene_ctx* const ctx)
{
	target->set_transform(initial);
}
nlohmann::json transform_action::save() const
{
	nlohmann::json obj;
	obj["type"] = 0;
	obj["t"] = target->id;
	obj["i"] = initial.e;
	obj["m"] = mat.e;
	return obj;
}



reparent_action::reparent_action(sgnode* const target, sgnode* const _new_parent, const s64 _new_index) :
	action(target),
	old_parent(target->parent),
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
void reparent_action::apply(scene_ctx* const ctx)
{
	old_index = old_parent->remove_child(target);
	new_parent->add_child(target, new_index);
}
void reparent_action::undo(scene_ctx* const ctx)
{
	new_parent->remove_child(target);
	old_parent->add_child(target, old_index);
}
nlohmann::json reparent_action::save() const
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



create_action::create_action(sgnode* const new_node, sgnode* const _parent) :
	action(new_node),
	parent(_parent)
{}
create_action::create_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes) :
	action(obj, nodes),
	parent(nodes.at(obj["p"]))
{}
void create_action::apply(scene_ctx* const ctx)
{
	parent->add_child(target);
}
void create_action::undo(scene_ctx* const ctx)
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
	obj["t"] = target->id;
	obj["p"] = parent->id;
	return obj;
}



destroy_action::destroy_action(sgnode* const target) :
	action(target),
	parent(target->parent),
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
void destroy_action::apply(scene_ctx* const ctx)
{
	if (parent)
	{
		index = parent->remove_child(target);
	}
}
void destroy_action::undo(scene_ctx* const ctx)
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
	obj["t"] = target->id;
	obj["p"] = parent->id;
	obj["i"] = index;
	return obj;
}



freeze_action::freeze_action(sgnode* const target) :
	action(target),
	frozen(target->freeze()),
	index(target->get_index())
{
	assert(!target->is_root());
}
freeze_action::freeze_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes) :
	action(obj, nodes),
	frozen(nodes.at(obj["f"])),
	index(obj["i"])
{}
void freeze_action::apply(scene_ctx* const ctx)
{
	target->parent->remove_child(target);
	target->parent->add_child(frozen, index);
}
void freeze_action::undo(scene_ctx* const ctx)
{
	target->parent->remove_child(frozen);
	target->parent->add_child(target, index);
}
bool freeze_action::redo_conflict(const sgnode* const selected) const
{
	return selected == target;
}
bool freeze_action::undo_conflict(const sgnode* const selected) const
{
	return selected == frozen;
}
nlohmann::json freeze_action::save() const
{
	nlohmann::json obj;
	obj["type"] = 4;
	obj["t"] = target->id;
	obj["f"] = frozen->id;
	obj["i"] = index;
	return obj;
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
void unfreeze_action::apply(scene_ctx* const ctx)
{
	target->parent->remove_child(target);
	target->parent->add_child(unfrozen, index);
	unfrozen->set_transform(target->mat);
}
void unfreeze_action::undo(scene_ctx* const ctx)
{
	target->parent->remove_child(unfrozen);
	target->parent->add_child(target, index);
	target->set_transform(unfrozen->mat);
}
bool unfreeze_action::redo_conflict(const sgnode* const selected) const
{
	return selected == target;
}
bool unfreeze_action::undo_conflict(const sgnode* const selected) const
{
	return selected == unfrozen;
}
nlohmann::json unfreeze_action::save() const
{
	nlohmann::json obj;
	obj["type"] = 4;
	obj["t"] = target->id;
	obj["u"] = unfrozen->id;
	obj["i"] = index;
	return obj;
}
