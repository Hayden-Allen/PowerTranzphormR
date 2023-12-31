#pragma once
#include "pch.h"

struct action;
struct app_ctx;
class scene_ctx;
class sgnode;

class action_stack
{
public:
	action_stack(scene_ctx* const sc, app_ctx* const ac);
	MGL_DCM(action_stack);
	virtual ~action_stack();
public:
	void transform(sgnode* const t, const tmat<space::OBJECT, space::PARENT>& old_mat, const tmat<space::OBJECT, space::PARENT>& new_mat);
	void reparent(sgnode* const target, sgnode* const new_parent, const s64 new_index, const u32 skip_count = 0);
	void create(sgnode* const target, sgnode* const parent, const u32 skip_count = 0);
	void destroy(sgnode* const target);
	sgnode* freeze(sgnode* const target);
	void unfreeze(sgnode* const target, sgnode* const unfrozen);
	void rename(sgnode* const target, const std::string& new_name);
	void set_operation(sgnode* const target, const carve::csg::CSG::OP new_op);
	bool can_undo();
	bool can_redo();
	action* undo();
	action* redo();
	void save(std::ofstream& out, const sgnode* const root) const;
	std::unordered_map<std::string, sgnode*> load(std::ifstream& in);
	void clear();
	bool get_modified() const;
private:
	scene_ctx* m_ctx;
	app_ctx* m_app_ctx;
	// ctrl-z (undo)
	std::vector<action*> m_past;
	// ctrl-y (redo)
	std::vector<action*> m_future;
	mutable bool m_modified = false;
private:
	// create and apply a new action
	void new_action(action* const a, const bool apply);
	void clear(sgnode* const node, std::unordered_set<sgnode*>& freed);
};
