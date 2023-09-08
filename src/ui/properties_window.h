#pragma once
#include "imgui_window.h"

class sgnode;
struct autotexture_params;
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
	void handle_material_texture(scene_material* const selected_mtl, const std::string& name, bool is_button);
	void handle_autotexture_server();
	void handle_material_autotexture(scene_material* const selected_mtl, const std::string& name);
	void handle_material_autotexture_generate(scene_material* const selected_mtl, const std::string& name);
private:
	void load_autotex_settings();
	void save_autotex_settings();
	bool autotex_fetch_post(const std::string& host, const std::string& path, const nlohmann::json& body, nlohmann::json& result, int expect_status = 200);
	void autotex_sampler_combo(autotexture_params& params, const std::string& sampler_name);
private:
	std::string m_autotex_url, m_autotex_username, m_autotex_password;
	bool m_autotex_needs_load = true;
};
