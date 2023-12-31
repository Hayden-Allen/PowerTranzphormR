#pragma once
#include "pch.h"

class scene_ctx;
struct app_ctx;
class sgnode;
class action_stack;

struct action
{
	friend class action_stack;
public:
	sgnode* target;
	u32 skip_count;
public:
	action(sgnode* const t, const u32 sc = 0);
	action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes);
	MGL_DCM(action);
	virtual ~action() {}
public:
	static action* create(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes);
public:
	virtual void apply(scene_ctx* const ctx, app_ctx* const a_ctx) = 0;
	virtual void undo(scene_ctx* const ctx, app_ctx* const a_ctx) = 0;
	virtual nlohmann::json save() const;
	virtual bool redo_conflict(const sgnode* const selected) const;
	virtual bool undo_conflict(const sgnode* const selected) const;
protected:
	virtual void all_nodes(std::unordered_set<const sgnode*>& nodes) const;
};

struct transform_action : public action
{
public:
	tmat<space::OBJECT, space::PARENT> initial;
	tmat<space::OBJECT, space::PARENT> mat;
public:
	transform_action(sgnode* const target, const tmat<space::OBJECT, space::PARENT>& old_mat, const tmat<space::OBJECT, space::PARENT>& new_mat);
	transform_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes);
	MGL_DCM(transform_action);
public:
	void apply(scene_ctx* const ctx, app_ctx* const a_ctx) override;
	void undo(scene_ctx* const ctx, app_ctx* const a_ctx) override;
	nlohmann::json save() const override;
};

struct reparent_action : public action
{
public:
	sgnode *old_parent, *new_parent;
	s64 old_index = -1, new_index = -1;
public:
	reparent_action(sgnode* const target, sgnode* const _new_parent, const s64 _new_index, const u32 _skip_count);
	reparent_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes);
	MGL_DCM(reparent_action);
public:
	void apply(scene_ctx* const ctx, app_ctx* const a_ctx) override;
	void undo(scene_ctx* const ctx, app_ctx* const a_ctx) override;
	nlohmann::json save() const override;
protected:
	void all_nodes(std::unordered_set<const sgnode*>& nodes) const override;
};

struct create_action : public action
{
public:
	sgnode* parent;
public:
	create_action(sgnode* const new_node, sgnode* const _parent, const u32 skip_count);
	create_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes);
	MGL_DCM(create_action);
public:
	void apply(scene_ctx* const ctx, app_ctx* const a_ctx) override;
	void undo(scene_ctx* const ctx, app_ctx* const a_ctx) override;
	bool redo_conflict(const sgnode* const selected) const;
	bool undo_conflict(const sgnode* const selected) const;
	nlohmann::json save() const override;
protected:
	void all_nodes(std::unordered_set<const sgnode*>& nodes) const override;
};

struct destroy_action : public action
{
public:
	sgnode* parent;
	s64 index;
public:
	destroy_action(sgnode* const target);
	destroy_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes);
	MGL_DCM(destroy_action);
public:
	void apply(scene_ctx* const ctx, app_ctx* const a_ctx) override;
	void undo(scene_ctx* const ctx, app_ctx* const a_ctx) override;
	bool redo_conflict(const sgnode* const selected) const;
	bool undo_conflict(const sgnode* const selected) const;
	nlohmann::json save() const override;
protected:
	void all_nodes(std::unordered_set<const sgnode*>& nodes) const override;
};

struct freeze_action : public action
{
public:
	sgnode* frozen;
	s64 index;
public:
	freeze_action(sgnode* const target, scene_ctx* const scene);
	freeze_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes);
	MGL_DCM(freeze_action);
public:
	void apply(scene_ctx* const ctx, app_ctx* const a_ctx) override;
	void undo(scene_ctx* const ctx, app_ctx* const a_ctx) override;
	nlohmann::json save() const override;
protected:
	void all_nodes(std::unordered_set<const sgnode*>& nodes) const override;
};

struct unfreeze_action : public action
{
public:
	sgnode* unfrozen;
	s64 index;
public:
	unfreeze_action(sgnode* const target, sgnode* const _unfrozen);
	unfreeze_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes);
	MGL_DCM(unfreeze_action);
public:
	void apply(scene_ctx* const ctx, app_ctx* const a_ctx) override;
	void undo(scene_ctx* const ctx, app_ctx* const a_ctx) override;
	nlohmann::json save() const override;
protected:
	void all_nodes(std::unordered_set<const sgnode*>& nodes) const override;
};

struct rename_action : public action
{
public:
	std::string old_name, new_name;
public:
	rename_action(sgnode* const target, const std::string& name);
	rename_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes);
	MGL_DCM(rename_action);
public:
	void apply(scene_ctx* const ctx, app_ctx* const a_ctx) override;
	void undo(scene_ctx* const ctx, app_ctx* const a_ctx) override;
	nlohmann::json save() const override;
};

struct set_op_action : public action
{
	typedef carve::csg::CSG::OP op_t;
public:
	op_t old_op, new_op;
public:
	set_op_action(sgnode* const target, const op_t newo);
	set_op_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes);
	MGL_DCM(set_op_action);
public:
	void apply(scene_ctx* const ctx, app_ctx* const a_ctx) override;
	void undo(scene_ctx* const ctx, app_ctx* const a_ctx) override;
	nlohmann::json save() const override;
};
