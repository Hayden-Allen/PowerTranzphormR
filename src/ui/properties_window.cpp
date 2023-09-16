#include "pch.h"
#include "properties_window.h"
#include "geom/generated_mesh.h"
#include "app_ctx.h"
#include "core/sgnode.h"
#include "core/smnode.h"
#include "core/scene_material.h"
#include "core/xportable.h"

properties_window::properties_window(app_ctx* const a_ctx) :
	imgui_window(a_ctx, "Properties")
{
	if (m_autotex_needs_load)
	{
		load_autotex_settings();
	}
}



void properties_window::handle_frame()
{
	ImGui::PushID("propswin");

	sgnode* const selected_sgnode = m_app_ctx->get_selected_sgnode();
	scene_material* const selected_material = m_app_ctx->get_selected_material();
	light* const selected_light = m_app_ctx->get_selected_light();
	waypoint* const selected_waypoint = m_app_ctx->get_selected_waypoint();
	smnode* const selected_static_mesh = m_app_ctx->get_selected_static_mesh();

	if (selected_sgnode)
	{
		if (selected_sgnode != m_prev_xportable)
		{
			m_needs_extract_transform = true;
		}
		ImGui::PushID(selected_sgnode->get_id().c_str());
		handle_sgnode_frame(selected_sgnode);
		ImGui::PopID();
	}
	else if (selected_material)
	{
		if (selected_material != m_prev_xportable)
		{
			m_needs_extract_transform = true;
		}
		ImGui::PushID(selected_material->get_id().c_str());
		handle_material_frame(selected_material);
		ImGui::PopID();
	}
	else if (selected_light)
	{
		if (selected_light != m_prev_xportable)
		{
			m_needs_extract_transform = true;
		}
		ImGui::PushID(selected_light->get_id().c_str());
		handle_light_frame(selected_light);
		ImGui::PopID();
	}
	else if (selected_waypoint)
	{
		if (selected_waypoint != m_prev_xportable)
		{
			m_needs_extract_transform = true;
		}
		ImGui::PushID(selected_waypoint->get_id().c_str());
		handle_waypoint_frame(selected_waypoint);
		ImGui::PopID();
	}
	else if (selected_static_mesh)
	{
		if (selected_static_mesh != m_prev_xportable)
		{
			m_needs_extract_transform = true;
		}
		ImGui::PushID(selected_static_mesh->get_id().c_str());
		handle_static_mesh_frame(selected_static_mesh);
		ImGui::PopID();
	}
	else
	{
		m_prev_xportable = nullptr;
	}

	ImGui::PopID();
}
void properties_window::handle_sgnode_frame(sgnode* const selected)
{
	handle_xportable(selected);

	if (handle_transform(selected->get_mat().e))
	{
		selected->set_transform(selected->get_mat().e);
	}

	if (selected->is_root())
	{
		handle_sgnode_camera_light();
		handle_sgnode_snapping_angle();
		handle_sgnode_skybox();
	}

	if (!selected->is_operation())
	{
		handle_sgnode_mesh(selected);
	}
}
void properties_window::handle_sgnode_camera_light()
{
}
void properties_window::handle_sgnode_snapping_angle()
{
	ImGui::SeparatorText("Shading");

	const bool start_all = scene_ctx::s_snap_all;
	// ImGui::Checkbox("Snap All", &scene_ctx::s_snap_all);
	if (handle_snap_mode(start_all))
		scene_ctx::s_snap_all ^= 1;

	const f32 start_angle = scene_ctx::s_snap_angle;
	ImGui::SliderAngle("##Snap Angle", &scene_ctx::s_snap_angle, 0.f, 360.f);

	if (start_angle != scene_ctx::s_snap_angle || start_all != scene_ctx::s_snap_all)
		m_app_ctx->scene.get_sg_root()->set_dirty();
}
void properties_window::handle_sgnode_skybox()
{
	ImGui::SeparatorText("Skybox");

	if (ImGui::Button("Load Skybox"))
	{
		const std::string folder = u::open_folder(m_app_ctx->mgl_ctx.window);
		if (!folder.empty())
		{
			m_app_ctx->scene.load_skybox(folder, "");
		}
	}
}
bool properties_window::handle_snap_mode(const bool value)
{
	const std::string snap_text[2] = {
		"Snap normals if ANY within",
		"Snap normals if ALL within",
	};
	bool result = false;
	if (ImGui::BeginCombo("##Condition", snap_text[value].c_str()))
	{
		for (u32 i = 0; i < 2; i++)
		{
			const bool selected = value == (bool)i;
			if (ImGui::Selectable(snap_text[i].c_str(), &selected))
			{
				if (value != (bool)i)
					result = true;
			}
		}
		ImGui::EndCombo();
	}
	return result;
}
bool properties_window::handle_transform(f32* const elements)
{
	if (m_needs_extract_transform) // true when selection has changed
	{
		ImGuizmo::DecomposeMatrixToComponents(elements, m_transform_pos, m_transform_rot, m_transform_scale);
		m_needs_extract_transform = false;
	}
	else // check if something else something else (e.g. gizmo) modified the matrix even though selection didn't change
	{
		f32 orig_recomposed[16] = { 0.0f };
		ImGuizmo::RecomposeMatrixFromComponents(m_transform_pos, m_transform_rot, m_transform_scale, orig_recomposed);
		for (s32 i = 0; i < 16; ++i)
		{
			if (fabsf(elements[i] - orig_recomposed[i]) > FLT_EPSILON * 2.0f)
			{
				ImGuizmo::DecomposeMatrixToComponents(elements, m_transform_pos, m_transform_rot, m_transform_scale);
			}
		}
	}

	// actual UI and recompose the new matrix after change
	bool dirty = false;
	ImGui::SeparatorText("Transform");
	if (ImGui::DragFloat3("Position", m_transform_pos, 0.01f))
	{
		dirty = true;
	}
	if (ImGui::DragFloat3("Rotation", m_transform_rot, 0.2f))
	{
		dirty = true;
	}
	if (ImGui::DragFloat3("Scale", m_transform_scale, 0.01f, 0.001f, MAX_VALUE_TYPE(f32)))
	{
		dirty = true;
	}
	if (dirty)
	{
		ImGuizmo::RecomposeMatrixFromComponents(m_transform_pos, m_transform_rot, m_transform_scale, elements);
	}
	return dirty;
}
void properties_window::handle_xportable(xportable* x)
{
	if (x != m_prev_xportable)
	{
		m_cur_tag_input = "";
		m_prev_xportable = x;
	}


	ImGui::SeparatorText("Identifiers");

	std::string s = x->get_kustom_display();
	if (ImGui::InputText("ID", &s))
	{
		x->kustomize_display(s);
	}
	if (x->get_kustom_id_conflict())
	{
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Invalid ID - must be unique");
	}

	const f32 orig_avail = ImGui::GetWindowContentRegionWidth();
	f32 cur_avail = orig_avail;
	bool is_first = true;
	xportable::tag to_erase = { "", 0 };
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 16.f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.f, ImGui::GetStyle().FramePadding.y));
	for (const auto& s : x->get_tagz())
	{
		const f32 cur_width = ImGui::GetIO().Fonts->Fonts[0]->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, 0.0f, s.name.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f + ImGui::GetStyle().ItemSpacing.x;
		cur_avail -= cur_width;
		if (cur_avail > 0.0f)
		{
			if (!is_first)
			{
				ImGui::SameLine();
			}
		}
		else
		{
			cur_avail = orig_avail - cur_width;
		}
		is_first = false;

		const f32 hue = 240.f * (s.id % 16) / 16 + 120.f;
		ImGui::PushStyleColor(ImGuiCol_Button, u::hsl2rgb(hue, .5f, .45f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, u::hsl2rgb(hue, .5f, .55f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, u::hsl2rgb(hue, .5f, .55f));
		if (ImGui::Button(s.name.c_str()) || ImGui::IsItemClicked(ImGuiMouseButton_Middle))
		{
			to_erase = s;
		}
		ImGui::PopStyleColor(3);
	}
	ImGui::PopStyleVar(2);
	if (!to_erase.name.empty())
	{
		x->erase_tag(to_erase);
	}
	const bool pressed_enter = ImGui::InputText("Add Tag", &m_cur_tag_input, ImGuiInputTextFlags_EnterReturnsTrue);
	if (!m_cur_tag_input.empty())
	{
		if (pressed_enter)
		{
			x->push_tag(m_cur_tag_input);
			m_cur_tag_input = "";
			ImGui::SetKeyboardFocusHere(-1);
		}
		else
		{
			const auto& suggestions = xportable::get_tag_suggestions(m_cur_tag_input);
			const bool is_input_text_active = ImGui::IsItemActive() || ImGui::IsItemActivated();
			if (is_input_text_active && suggestions.size() > 0)
				ImGui::OpenPopup("##suggestions");
			ImGui::SetNextWindowPos(ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y));
			if (ImGui::BeginPopup("##suggestions", ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_ChildWindow))
			{
				for (const auto& s : suggestions)
				{
					if (ImGui::Selectable(s))
					{
						x->push_tag(s);
						m_cur_tag_input = "";
					}
				}

				if (pressed_enter || (!is_input_text_active && !ImGui::IsWindowFocused()))
					ImGui::CloseCurrentPopup();

				ImGui::EndPopup();
			}
		}
	}
}
void properties_window::handle_sgnode_mesh(sgnode* const selected)
{
	const u32 selected_mtl_id = selected->get_material();
	if (!selected->is_operation() && selected_mtl_id != MAX_VALUE(selected_mtl_id))
	{
		const u32 new_mtl_id = material_combo_box(selected_mtl_id);
		if (new_mtl_id != selected_mtl_id)
			m_app_ctx->set_material(selected, new_mtl_id);
	}

	if (selected->is_frozen())
	{
		ImGui::SeparatorText("Phrozen Mesh");

		if (ImGui::Button("Center at Origin"))
		{
			selected->center_frozen_vertices_at_origin();
		}

		ImGui::ColorEdit4("##RESETVTXCOL", m_reset_vertex_color);
		ImGui::SameLine();
		if (ImGui::Button("Reset All Vertex Colors"))
		{
			selected->set_frozen_vertices_color(color_t(m_reset_vertex_color[0], m_reset_vertex_color[1], m_reset_vertex_color[2], m_reset_vertex_color[3]), &m_app_ctx->scene);
		}
	}

	const bool changed = !selected->is_frozen() ? draw_params(selected->get_gen()->get_params()) : false;
	if (changed)
		selected->set_gen_dirty();
}
void properties_window::handle_material_frame(scene_material* const selected)
{
	handle_xportable(selected);

	ImGui::SeparatorText("Shading");

	bool should_cull = selected->get_should_cull();
	if (ImGui::Checkbox("Enable Back-face Culling", &should_cull))
	{
		selected->set_should_cull(should_cull);
	}
	bool use_lighting = selected->get_use_lighting();
	if (ImGui::Checkbox("Enable Lighting", &use_lighting))
	{
		selected->set_use_lighting(use_lighting);
	}
	bool use_alpha = selected->get_use_alpha();
	if (ImGui::Checkbox("Use Alpha Shader", &use_alpha))
	{
		selected->set_use_alpha(use_alpha);
	}

	ImGui::SeparatorText("Textures");

	selected->for_each_texture([&](const std::string& name, const mgl::texture2d_rgb_u8* tex_DONOTUSE)
		{
			if (ImGui::CollapsingHeader(name.c_str()))
			{
				ImGui::PushID(name.c_str());
				autotexture_params& at_params = selected->get_autotexture_params(name);
				bool at_enabled = at_params.enabled;
				if (ImGui::Checkbox("PowerTextuRe", &at_enabled))
				{
					if (m_app_ctx->loaded_filename.empty())
					{
						at_params.enabled = false;
						u::info_message_box(m_app_ctx->mgl_ctx.window, L"You must save the scene before enabling PowerTextuRe.", L"PowerTranzphormR - Error");
					}
					else
					{
						at_params.enabled = at_enabled;
					}
				}
				ImGui::SameLine();
				const f32 reset_width = 80.0f;
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - reset_width);
				if (ImGui::Button("Reset", ImVec2(reset_width, 0)))
				{
					at_params.enabled = false;
					selected->set_texture(name, g::null_tex_fp);
				}
				else
				{
					if (at_params.enabled)
					{
						handle_material_autotexture(selected, name);
						handle_material_texture(selected, name, false);
					}
					else
					{
						handle_material_texture(selected, name, true);
					}
				}
				ImGui::PopID();
			}
		});

	handle_autotexture_server();
}
void properties_window::handle_material_texture(scene_material* const selected_mtl, const std::string& name, bool is_button)
{
	const ImVec2 win_min = ImGui::GetWindowContentRegionMin(), win_max = ImGui::GetWindowContentRegionMax();
	const f32 win_w = win_max.x - win_min.x;
	const f32 img_w = std::min(win_w, m_app_ctx->mgl_ctx.get_height() * 0.3f);
	const ImVec2 img_dim(img_w, img_w);
	ImGui::SetCursorPosX(win_min.x + (win_w - img_dim.x) * 0.5f);
	if (is_button)
	{
		if (ImGui::ImageButton("imgbtn", selected_mtl->get_texture(name)->get_imgui_id(), img_dim, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f)))
		{
			const std::string& fp = u::open_dialog(m_app_ctx->mgl_ctx.window, "Image File", "png,jpg,bmp");
			if (!fp.empty())
				selected_mtl->set_texture(name, fp);
		}
	}
	else
	{
		ImGui::Image(selected_mtl->get_texture(name)->get_imgui_id(), img_dim, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
	}
}
void properties_window::handle_autotexture_server()
{
	if (!ImGui::CollapsingHeader("PowerTextuRe Settings"))
	{
		return;
	}

	ImGui::PushID("PowerTextuReConfig");
	bool needs_save = false;
	needs_save = ImGui::InputText("URL", &m_autotex_url);
	needs_save = ImGui::InputText("Username", &m_autotex_username) || needs_save;
	needs_save = ImGui::InputText("Password", &m_autotex_password, ImGuiInputTextFlags_Password) || needs_save;
	if (needs_save)
	{
		save_autotex_settings();
	}
	ImGui::PopID();
}
void properties_window::handle_material_autotexture(scene_material* const selected_mtl, const std::string& name)
{
	autotexture_params& at_params = selected_mtl->get_autotexture_params(name);
	ImGui::DragInt2("Size", at_params.dims, 1.0f, 0, 1024);
	ImGui::DragInt2("Resize", at_params.post_dims, 1.0f, 0, 1024);
	if (ImGui::BeginCombo("Sampler", at_params.sampler.c_str()))
	{
		autotex_sampler_combo(at_params, "DPM++ 2M Karras");
		autotex_sampler_combo(at_params, "DPM++ SDE Karras");
		autotex_sampler_combo(at_params, "DPM++ 2M SDE Exponential");
		autotex_sampler_combo(at_params, "DPM++ 2M SDE Karras");
		autotex_sampler_combo(at_params, "Euler a");
		autotex_sampler_combo(at_params, "Euler");
		autotex_sampler_combo(at_params, "LMS");
		autotex_sampler_combo(at_params, "Heun");
		autotex_sampler_combo(at_params, "DPM2");
		autotex_sampler_combo(at_params, "DPM2 a");
		autotex_sampler_combo(at_params, "DPM++ 2S a");
		autotex_sampler_combo(at_params, "DPM++ 2M");
		autotex_sampler_combo(at_params, "DPM++ SDE");
		autotex_sampler_combo(at_params, "DPM++ 2M SDE");
		autotex_sampler_combo(at_params, "DPM++ 2M SDE Heun");
		autotex_sampler_combo(at_params, "DPM++ 2M SDE Heun Karras");
		autotex_sampler_combo(at_params, "DPM++ 2M SDE Heun Exponential");
		autotex_sampler_combo(at_params, "DPM++ 3M SDE");
		autotex_sampler_combo(at_params, "DPM++ 3M SDE Karras");
		autotex_sampler_combo(at_params, "DPM++ 3M SDE Exponential");
		autotex_sampler_combo(at_params, "DPM fast");
		autotex_sampler_combo(at_params, "DPM adaptive");
		autotex_sampler_combo(at_params, "LMS Karras");
		autotex_sampler_combo(at_params, "DPM2 Karras");
		autotex_sampler_combo(at_params, "DPM2 a Karras");
		autotex_sampler_combo(at_params, "DPM++ 2S a Karras");
		autotex_sampler_combo(at_params, "Restart");
		autotex_sampler_combo(at_params, "DDIM");
		autotex_sampler_combo(at_params, "PLMS");
		autotex_sampler_combo(at_params, "UniPC");
		ImGui::EndCombo();
	}
	ImGui::DragInt("Seed", &at_params.seed, u::rand(99.0f, 1999.0f), 0, MAX_VALUE_TYPE(s32));
	ImGui::DragInt("Steps", &at_params.steps, 1.0f, 0, 100);
	ImGui::DragFloat("CFG Scale", &at_params.cfg_scale, 1.0f, 0.0f, 15.0f);
	ImGui::InputText("Prompt", &at_params.prompt);
	ImGui::InputText("Negative Prompt", &at_params.neg_prompt);
	ImGui::Checkbox("Tiling", &at_params.tiling);
	if (ImGui::Button("Randomize & Generate"))
	{
		at_params.seed = (s32)u::rand(0.0f, (f32)MAX_VALUE_TYPE(s32));
		handle_material_autotexture_generate(selected_mtl, name);
	}
	ImGui::SameLine();
	if (ImGui::Button("Generate"))
	{
		handle_material_autotexture_generate(selected_mtl, name);
	}
}
void properties_window::handle_light_frame(light* const selected)
{
	handle_xportable(selected);

	bool changed = false;
	const light_type type = selected->get_type();
	if (handle_transform(selected->get_mat().e))
	{
		changed = true;
		selected->set_mat(selected->get_mat());
	}
	// maintain type id in mat[3][3]
	selected->set_type(type);

	ImGui::SeparatorText("Light");
	ImGui::PushID(selected->get_id().c_str());
	if (ImGui::BeginCombo("Type", light_type_string(type).c_str()))
	{
		for (u32 i = 0; i < (u32)light_type::COUNT; i++)
		{
			light_type cur = (light_type)i;
			const bool cur_selected = type == cur;
			ImGui::PushID(i);
			if (ImGui::Selectable(light_type_string(cur).c_str(), cur_selected))
			{
				if (!cur_selected)
				{
					changed = true;
					selected->set_type(cur);
				}
			}
			if (cur_selected)
			{
				ImGui::SetItemDefaultFocus();
			}
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}
	ImGui::PopID();

	f32 last_tmin = -1.f, last_tmax = -1.f;
	if (selected->get_type() == light_type::SPOT)
	{
		last_tmin = selected->mgl_light.cos_tmin;
		last_tmax = selected->mgl_light.cos_tmax;
	}
	changed |= draw_params(selected->get_params());
	if (selected->get_type() == light_type::SPOT)
	{
		if (selected->mgl_light.cos_tmin != last_tmin)
			selected->mgl_light.cos_tmin = std::clamp(selected->mgl_light.cos_tmin, selected->mgl_light.cos_tmax + c::EPSILON, 1.f);
		if (selected->mgl_light.cos_tmax != last_tmax)
			selected->mgl_light.cos_tmax = std::clamp(selected->mgl_light.cos_tmax, 0.f, selected->mgl_light.cos_tmin - c::EPSILON);
	}
	if (changed)
	{
		m_app_ctx->scene.update_light(selected);
	}
}
void properties_window::handle_waypoint_frame(waypoint* const selected)
{
	handle_xportable(selected);
	handle_transform(selected->get_mat().e);
}
void properties_window::handle_static_mesh_frame(smnode* const selected)
{
	handle_xportable(selected);
	handle_transform(selected->get_mat().e);

	const u32 old_mtl_id = selected->get_material();
	const u32 new_mtl_id = material_combo_box(old_mtl_id);
	if (new_mtl_id != old_mtl_id)
	{
		selected->set_material(&m_app_ctx->scene, new_mtl_id);
	}
	bool snap = selected->should_snap();
	if (ImGui::Checkbox("Snap Normals", &snap))
	{
		selected->set_should_snap(snap);
	}
	bool snap_all = selected->should_snap_all();
	if (handle_snap_mode(snap_all))
	{
		snap_all ^= 1;
		selected->set_should_snap_all(snap_all);
	}

	f32 snap_angle = selected->get_snap_angle();
	if (ImGui::SliderAngle("##Snap Angle", &snap_angle, 0.f, 360.f))
	{
		selected->set_snap_angle(snap_angle);
	}

	draw_params(selected->get_params());

	if (selected->is_static())
	{
		ImGui::SeparatorText("Static Mesh");

		if (ImGui::Button("Center at Origin"))
		{
			selected->center_vertices_at_origin();
		}

		ImGui::ColorEdit4("##RESETVTXCOL", m_reset_vertex_color);
		ImGui::SameLine();
		if (ImGui::Button("Reset All Vertex Colors"))
		{
			selected->set_vertices_color(color_t(m_reset_vertex_color[0], m_reset_vertex_color[1], m_reset_vertex_color[2], m_reset_vertex_color[3]), &m_app_ctx->scene);
		}
	}
	else
	{
		ImGui::SeparatorText("Heightmap");
		if (ImGui::Button("Load Heightmap"))
		{
			const std::string& fp = u::open_dialog(m_app_ctx->mgl_ctx.window, "Image File", "png,jpg,bmp");
			if (!fp.empty())
			{
				const u32 material = selected->get_material();
				const mgl::retained_texture2d_rgba_u8* tex = u::load_retained_texture2d_rgba_u8(fp);
				const heightmap_options new_hm_opts = { .map = tex };
				generated_mesh* const new_hm = m_app_ctx->scene.generated_textured_heightmap_static(material, new_hm_opts);
				selected->set_gen(new_hm);
			}
		}
	}
}
u32 properties_window::material_combo_box(const u32 selected)
{
	ImGui::SeparatorText("Shading");
	u32 new_selected = selected;
	if (ImGui::BeginCombo("Material", m_app_ctx->scene.get_material(selected)->get_name().c_str()))
	{
		const auto& sorted_mtls = m_app_ctx->get_sorted_materials();
		for (const auto& pair : sorted_mtls)
		{
			const bool cur_mtl_combo_selected = pair.first == selected;
			ImGui::PushID(pair.first);
			if (ImGui::Selectable(pair.second->get_name().c_str(), cur_mtl_combo_selected))
			{
				new_selected = pair.first;
			}
			if (cur_mtl_combo_selected)
			{
				ImGui::SetItemDefaultFocus();
			}
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}
	return new_selected;
}
bool properties_window::draw_params(const std::vector<std::pair<std::string, generated_mesh_param>>& params)
{
	ImGui::SeparatorText("Parameters");

	bool changed = false;
	for (const auto& prop : params)
	{
		switch (prop.second.type)
		{
		case generated_mesh_param_type::UINT_1:
			changed |= ImGui::DragInt(prop.first.c_str(), static_cast<int*>(prop.second.value), prop.second.speed, static_cast<int>(prop.second.min), static_cast<int>(prop.second.max), "%d", ImGuiSliderFlags_AlwaysClamp);
			break;
		case generated_mesh_param_type::UINT_1_LOG:
			changed |= ImGui::DragInt(prop.first.c_str(), static_cast<int*>(prop.second.value), prop.second.speed, static_cast<int>(prop.second.min), static_cast<int>(prop.second.max), "%d", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);
			break;
		case generated_mesh_param_type::UINT_2:
			changed |= ImGui::DragInt2(prop.first.c_str(), static_cast<int*>(prop.second.value), prop.second.speed, static_cast<int>(prop.second.min), static_cast<int>(prop.second.max), "%d", ImGuiSliderFlags_AlwaysClamp);
			break;
		case generated_mesh_param_type::UINT_2_LOG:
			changed |= ImGui::DragInt2(prop.first.c_str(), static_cast<int*>(prop.second.value), prop.second.speed, static_cast<int>(prop.second.min), static_cast<int>(prop.second.max), "%d", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);
			break;
		case generated_mesh_param_type::FLOAT_1:
			changed |= ImGui::DragFloat(prop.first.c_str(), static_cast<float*>(prop.second.value), prop.second.speed, prop.second.min, prop.second.max, "%.3f", ImGuiSliderFlags_AlwaysClamp);
			break;
		case generated_mesh_param_type::FLOAT_1_LOG:
			changed |= ImGui::DragFloat(prop.first.c_str(), static_cast<float*>(prop.second.value), prop.second.speed, prop.second.min, prop.second.max, "%.3f", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);
			break;
		case generated_mesh_param_type::FLOAT_2:
			changed |= ImGui::DragFloat2(prop.first.c_str(), static_cast<float*>(prop.second.value), prop.second.speed, prop.second.min, prop.second.max, "%.3f", ImGuiSliderFlags_AlwaysClamp);
			break;
		case generated_mesh_param_type::FLOAT_4:
			changed |= ImGui::DragFloat4(prop.first.c_str(), static_cast<float*>(prop.second.value), prop.second.speed, prop.second.min, prop.second.max, "%.3f", ImGuiSliderFlags_AlwaysClamp);
			break;
		case generated_mesh_param_type::FLOAT_4_SUM_1:
			{
				f32* const f = static_cast<f32*>(prop.second.value);
				const f32 of[4] = { f[0], f[1], f[2], f[3] };
				changed |= ImGui::DragFloat4(prop.first.c_str(), f, prop.second.speed, prop.second.min, prop.second.max, "%.3f", ImGuiSliderFlags_AlwaysClamp);
				if (changed)
				{
					// index of weight that changed
					u32 ci = 0;
					for (u32 i = 1; i < 4; i++)
						if (f[i] != of[i])
							ci = i;

					// if weights were some permutation of (1, 0, 0, 0) and the 1 is being decreased,
					// bump all the other ones up equally to make up the difference
					if (of[ci] == 1.f)
					{
						for (u32 i = 0; i < 4; i++)
							if (i != ci)
								f[i] = (1.f - f[ci]) / 3;
						goto rebalance_end;
					}

					// sum of original values of non-changed weights
					f32 odenom = 0.f;
					for (u32 i = 0; i < 4; i++)
						if (i != ci)
							odenom += of[i];

					// original percentage of each weight
					f32 opct[4] = { 0.f };
					for (u32 i = 0; i < 4; i++)
						opct[i] = of[i] / odenom;

					// new sum of unchanged weights
					const f32 denom = 1.f - f[ci];
					// replace unchanged weights to maintain sum of 1
					for (u32 i = 0; i < 4; i++)
						if (i != ci)
							f[i] = opct[i] * denom;
				}
rebalance_end:
				break;
			}
		case generated_mesh_param_type::COLOR_4:
			changed |= ImGui::ColorEdit4(prop.first.c_str(), static_cast<float*>(prop.second.value));
			break;
		case generated_mesh_param_type::UINT_2_DIRECT:
			{
				s32* const v = static_cast<s32*>(prop.second.value);
				changed |= ImGui::InputInt2(prop.first.c_str(), v, ImGuiInputTextFlags_EnterReturnsTrue);
				v[0] = std::clamp(v[0], (s32)prop.second.min, (s32)prop.second.max);
				v[1] = std::clamp(v[1], (s32)prop.second.min, (s32)prop.second.max);
			}
			break;
		default:
			assert(false); // Unsupported type
			break;
		}
	}
	return changed;
}
void properties_window::handle_material_autotexture_generate(scene_material* const selected_mtl, const std::string& name)
{
	const autotexture_params& at_params = selected_mtl->get_autotexture_params(name);
	nlohmann::json img_req;
	img_req["seed"] = at_params.seed;
	img_req["steps"] = at_params.steps;
	img_req["sampler_index"] = at_params.sampler;
	img_req["cfg_scale"] = at_params.cfg_scale;
	img_req["prompt"] = at_params.prompt;
	img_req["negative_prompt"] = at_params.neg_prompt;
	img_req["width"] = at_params.dims[0];
	img_req["height"] = at_params.dims[1];
	img_req["tiling"] = at_params.tiling;
	nlohmann::json img_res;
	if (!autotex_fetch_post(m_autotex_url, "/sdapi/v1/txt2img", img_req, img_res))
	{
		u::error_message_box(m_app_ctx->mgl_ctx.window, L"Autotexture failed (see console output for more details)", L"PowerTranzphormR - Error");
		return;
	}
	std::string b64_decoded = std::move(base64_decode(img_res["images"][0]));
	const auto& lfn = m_app_ctx->loaded_filename;
	assert(!lfn.empty());
	std::filesystem::path lfp(lfn);
	assert(lfp.has_parent_path());
	std::filesystem::path lfpp = lfp.parent_path();
	std::filesystem::path outdirp = lfpp / "_PowerTextuRe_";
	std::filesystem::create_directory(outdirp);

	stbi_set_flip_vertically_on_load(true);
	int src_w = -1, src_h = -1, src_c = -1;
	stbi_uc* const src_data = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(b64_decoded.data()), (s32)b64_decoded.size(), &src_w, &src_h, &src_c, 3);
	assert(src_c >= 3);
	int dst_w = at_params.post_dims[0], dst_h = at_params.post_dims[1];
	stbi_uc* const dst_data = new stbi_uc[dst_w * dst_h * 3];
	stbir_resize_uint8(src_data, src_w, src_h, 0, dst_data, dst_w, dst_h, 0, 3);
	stbi_image_free(src_data);
	s32 cur_sec = (s32)std::floor(m_app_ctx->mgl_ctx.time.now);
	std::filesystem::path outp = outdirp / (std::to_string(cur_sec) + ".png");
	stbi_write_png(outp.string().c_str(), dst_w, dst_h, 3, dst_data, 0);
	delete[] dst_data;

	selected_mtl->set_texture(name, outp.string());
}
void properties_window::load_autotex_settings()
{
	m_autotex_needs_load = false;

	std::ifstream inf("autotexture.config.json");
	if (!inf)
	{
		return;
	}

	try
	{
		nlohmann::json j = nlohmann::json::parse(inf);
		m_autotex_url = j["url"];
		m_autotex_username = j["username"];
		m_autotex_password = j["password"];
	}
	catch (const std::exception&)
	{
		std::cerr << "Unable to parse autotexture configuration file\n";
	}
}
void properties_window::save_autotex_settings()
{
	std::ofstream outf("autotexture.config.json");
	nlohmann::json j;
	j["url"] = m_autotex_url;
	j["username"] = m_autotex_username;
	j["password"] = m_autotex_password;
	outf << j.dump() << "\n";
}
bool properties_window::autotex_fetch_post(const std::string& host, const std::string& path, const nlohmann::json& body, nlohmann::json& result, int expect_status)
{
	httplib::Client cli(host);
	cli.set_connection_timeout(120, 0);
	cli.set_read_timeout(120, 0);
	cli.set_write_timeout(120, 0);
	cli.set_follow_location(true);
	cli.set_basic_auth(m_autotex_username, m_autotex_password);
	httplib::Result res = cli.Post(path, body.dump(), "application/json");
	if (res)
	{
		if (res->status == expect_status)
		{
			result = std::move(nlohmann::json::parse(res->body));
			return true;
		}
		std::cout << "Error: Received status " << res->status << " from server: " << host << path << "\n";
		return false;
	}
	else
	{
		std::cout << "Error: Unable to contact server: " << host << path << "\n";
		return false;
	}
}
void properties_window::autotex_sampler_combo(autotexture_params& params, const std::string& sampler_name)
{
	bool selected = params.sampler == sampler_name;
	if (ImGui::Selectable(sampler_name.c_str(), &selected))
	{
		params.sampler = sampler_name;
	}
}
