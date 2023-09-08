#pragma once
#include "imgui_window.h"
#include "geom/generated_mesh.h"

class sgnode;
struct scene_material;
struct light;
class generated_mesh;

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
	void handle_material_texture(scene_material* const selected_mtl, const std::string& name, bool is_button);
	void handle_material_autotexture(scene_material* const selected_mtl, const std::string& name);
private:
	void handle_light_frame(light* const selected);
	void handle_static_mesh_frame(generated_mesh* const selected);
	u32 material_combo_box(const u32 selected);
	bool draw_params(const std::vector<std::pair<std::string, generated_mesh_param>>& params);
};
