#pragma once
#include "imgui_window.h"

class sgnode;
struct scene_material;

class properties_window : public imgui_window
{
public:
	properties_window(app_ctx* const a_ctx);
	virtual ~properties_window() {}
public:
	virtual void handle_frame() override;
private:
	void handle_sgnode_frame(sgnode* const selected);
	void handle_sgnode_snapping_angle();
	void handle_sgnode_transform(sgnode* const selected);
	void handle_sgnode_mesh(sgnode* const selected);
private:
	void handle_material_frame(scene_material* const selected);
	void handle_material_texture(scene_material* const selected_mtl, const std::string& name);
};
