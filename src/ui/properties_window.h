#pragma once
#include "imgui_window.h"
#include "geom/generated_mesh.h"

class sgnode;
struct autotexture_params;
struct scene_material;
class light;
class waypoint;
class generated_mesh;
class smnode;
class xportable;

class properties_window : public imgui_window
{
public:
	properties_window(app_ctx* const a_ctx);
	virtual ~properties_window() {}
public:
	virtual void handle_frame() override;
private:
	xportable* m_prev_xportable = nullptr;
	std::string m_cur_tag_input;
	std::string m_autotex_url, m_autotex_username, m_autotex_password;
	bool m_autotex_needs_load = true;
	float m_transform_pos[3] = { 0.0f }, m_transform_rot[3] = { 0.0f }, m_transform_scale[3] = { 0.0f };
	bool m_needs_extract_transform = true;
private:
	void handle_sgnode_frame(sgnode* const selected);
	void handle_sgnode_camera_light();
	void handle_sgnode_snapping_angle();
	void handle_sgnode_skybox();
	bool handle_snap_mode(const bool value);
	bool handle_transform(f32* const elements);
	void handle_xportable(xportable* x);
	void handle_sgnode_mesh(sgnode* const selected);
private:
	void handle_material_frame(scene_material* const selected);
	void handle_material_texture(scene_material* const selected_mtl, const std::string& name, bool is_button);
	void handle_autotexture_server();
	void handle_material_autotexture(scene_material* const selected_mtl, const std::string& name);
private:
	void handle_light_frame(light* const selected);
	void handle_waypoint_frame(waypoint* const selected);
	void handle_static_mesh_frame(smnode* const selected);
	u32 material_combo_box(const u32 selected);
	bool draw_params(const std::vector<std::pair<std::string, generated_mesh_param>>& params);
	void handle_material_autotexture_generate(scene_material* const selected_mtl, const std::string& name);
private:
	void load_autotex_settings();
	void save_autotex_settings();
	bool autotex_fetch_post(const std::string& host, const std::string& path, const nlohmann::json& body, nlohmann::json& result, int expect_status = 200);
	void autotex_sampler_combo(autotexture_params& params, const std::string& sampler_name);
};
