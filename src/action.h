#pragma once
#include "pch.h"
#include "scene_ctx.h"
#include "scene_graph.h"

struct action
{
public:
	sgnode* target;
public:
	action(sgnode* const t);
	action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes);
	MGL_DCM(action);
	virtual ~action() {}
public:
	static action* create(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes);
public:
	virtual void apply(scene_ctx* const ctx) = 0;
	virtual void undo(scene_ctx* const ctx) = 0;
	virtual nlohmann::json save() const = 0;
	virtual bool redo_conflict(const sgnode* const selected) const;
	virtual bool undo_conflict(const sgnode* const selected) const;
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
	void apply(scene_ctx* const ctx) override;
	void undo(scene_ctx* const ctx) override;
	nlohmann::json save() const override;
};

struct reparent_action : public action
{
public:
	sgnode *old_parent, *new_parent;
	s64 old_index = -1, new_index = -1;
public:
	reparent_action(sgnode* const target, sgnode* const _new_parent, const s64 _new_index);
	reparent_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes);
	MGL_DCM(reparent_action);
public:
	void apply(scene_ctx* const ctx) override;
	void undo(scene_ctx* const ctx) override;
	nlohmann::json save() const override;
};

struct create_action : public action
{
public:
	sgnode* parent;
public:
	create_action(sgnode* const new_node, sgnode* const _parent);
	create_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes);
	MGL_DCM(create_action);
public:
	void apply(scene_ctx* const ctx) override;
	void undo(scene_ctx* const ctx) override;
	bool redo_conflict(const sgnode* const selected) const;
	bool undo_conflict(const sgnode* const selected) const;
	nlohmann::json save() const override;
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
	void apply(scene_ctx* const ctx) override;
	void undo(scene_ctx* const ctx) override;
	bool redo_conflict(const sgnode* const selected) const;
	bool undo_conflict(const sgnode* const selected) const;
	nlohmann::json save() const override;
};

struct freeze_action : public action
{
public:
	sgnode* frozen;
	s64 index;
public:
	freeze_action(sgnode* const target);
	freeze_action(const nlohmann::json& obj, const std::unordered_map<std::string, sgnode*>& nodes);
	MGL_DCM(freeze_action);
public:
	void apply(scene_ctx* const ctx) override;
	void undo(scene_ctx* const ctx) override;
	bool redo_conflict(const sgnode* const selected) const;
	bool undo_conflict(const sgnode* const selected) const;
	nlohmann::json save() const override;
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
	void apply(scene_ctx* const ctx) override;
	void undo(scene_ctx* const ctx) override;
	bool redo_conflict(const sgnode* const selected) const;
	bool undo_conflict(const sgnode* const selected) const;
	nlohmann::json save() const override;
};