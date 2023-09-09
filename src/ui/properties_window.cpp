#include "pch.h"
#include "properties_window.h"
#include "geom/generated_mesh.h"
#include "app_ctx.h"
#include "core/sgnode.h"
#include "core/smnode.h"
#include "core/scene_material.h"

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
	sgnode* const selected_sgnode = m_app_ctx->get_selected_sgnode();
	if (selected_sgnode)
		handle_sgnode_frame(selected_sgnode);

	scene_material* const selected_material = m_app_ctx->get_selected_material();
	if (selected_material)
		handle_material_frame(selected_material);

	light* const selected_light = m_app_ctx->get_selected_light();
	if (selected_light)
		handle_light_frame(selected_light);

	smnode* const selected_static_mesh = m_app_ctx->get_selected_static_mesh();
	if (selected_static_mesh)
		handle_static_mesh_frame(selected_static_mesh);
}
void properties_window::handle_sgnode_frame(sgnode* const selected)
{
	if (selected->is_root())
	{
		handle_sgnode_snapping_angle();
	}
	const bool changed = handle_transform(selected->get_mat().e);
	if (changed)
	{
		selected->set_transform(selected->get_mat().e);
	}

	if (!selected->is_operation())
	{
		handle_sgnode_mesh(selected);
	}
}
void properties_window::handle_sgnode_snapping_angle()
{
	const bool start_all = scene_ctx::s_snap_all;
	ImGui::Checkbox("Snap All", &scene_ctx::s_snap_all);
	const f32 start_angle = scene_ctx::s_snap_angle;
	ImGui::SliderAngle("Snap Angle", &scene_ctx::s_snap_angle, 0.f, 360.f);

	if (start_angle != scene_ctx::s_snap_angle || start_all != scene_ctx::s_snap_all)
		m_app_ctx->scene.get_sg_root()->set_dirty();
}
bool properties_window::handle_transform(f32* const elements)
{
	bool dirty = false;
	float trans[3] = { 0.f }, rot[3] = { 0.f }, scale[3] = { 0.f };
	ImGuizmo::DecomposeMatrixToComponents(elements, trans, rot, scale);
	if (ImGui::DragFloat3("Position", trans, 0.01f))
	{
		dirty = true;
	}
	if (ImGui::DragFloat3("Rotation", rot, 0.2f))
	{
		dirty = true;
	}
	if (ImGui::DragFloat3("Scale", scale, 0.01f))
	{
		dirty = true;
	}
	if (dirty)
	{
		ImGuizmo::RecomposeMatrixFromComponents(trans, rot, scale, elements);
	}
	return dirty;
}
void properties_window::handle_sgnode_mesh(sgnode* const selected)
{
	if (!selected->is_operation() && !selected->is_frozen())
	{
		const u32 selected_mtl_id = selected->get_material();
		const u32 new_mtl_id = material_combo_box(selected_mtl_id);
		if (new_mtl_id != selected_mtl_id)
			m_app_ctx->set_material(selected, new_mtl_id);
	}

	const bool changed = draw_params(selected->get_gen()->get_params());
	if (changed)
		selected->set_gen_dirty();
}
void properties_window::handle_material_frame(scene_material* const selected)
{
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
	ImGui::DragInt2("Size", at_params.dims, 1.0f, 0, 512);
	if (ImGui::BeginCombo("Sampler", at_params.sampler.c_str()))
	{
		autotex_sampler_combo(at_params, "Euler");
		ImGui::EndCombo();
	}
	ImGui::DragInt("Seed", &at_params.seed, u::rand(99.0f, 1999.0f), 0, MAX_VALUE_TYPE(s32));
	ImGui::DragInt("Steps", &at_params.steps, 1.0f, 0, 100);
	ImGui::DragFloat("CFG Scale", &at_params.cfg_scale, 1.0f, 0.0f, 15.0f);
	ImGui::InputText("Prompt", &at_params.prompt);
	ImGui::InputText("Negative Prompt", &at_params.neg_prompt);
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
	bool changed = false;
	changed |= handle_transform(selected->get_mat().e);
	changed |= draw_params(selected->get_params());
	m_app_ctx->scene.update_light(selected);
}
void properties_window::handle_static_mesh_frame(smnode* const selected)
{
	if (!selected->is_static())
	{
		const u32 selected_mtl_id = selected->get_material();
		const u32 new_mtl_id = material_combo_box(selected_mtl_id);
		if (new_mtl_id != selected_mtl_id)
			selected->set_material(&m_app_ctx->scene, new_mtl_id);

		const bool changed = draw_params(selected->get_params());
		if (changed)
			selected->set_gen_dirty();
	}
}
u32 properties_window::material_combo_box(const u32 selected)
{
	u32 new_selected = selected;
	if (ImGui::BeginCombo("Material", m_app_ctx->scene.get_material(selected)->name.c_str()))
	{
		const auto& sorted_mtls = m_app_ctx->get_sorted_materials();
		for (const auto& pair : sorted_mtls)
		{
			bool cur_mtl_combo_selected = pair.first == selected;
			ImGui::PushID(pair.first);
			if (ImGui::Selectable(pair.second->name.c_str(), cur_mtl_combo_selected))
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
	bool changed = false;
	for (const auto& prop : params)
	{
		switch (prop.second.type)
		{
		case generated_mesh_param_type::UINT_1:
			changed |= ImGui::DragInt(prop.first.c_str(), static_cast<int*>(prop.second.value), prop.second.speed, static_cast<int>(prop.second.min), static_cast<int>(prop.second.max));
			break;
		case generated_mesh_param_type::UINT_2:
			changed |= ImGui::DragInt2(prop.first.c_str(), static_cast<int*>(prop.second.value), prop.second.speed, static_cast<int>(prop.second.min), static_cast<int>(prop.second.max));
			break;
		case generated_mesh_param_type::FLOAT_1:
			changed |= ImGui::DragFloat(prop.first.c_str(), static_cast<float*>(prop.second.value), prop.second.speed, prop.second.min, prop.second.max);
			break;
		case generated_mesh_param_type::FLOAT_2:
			changed |= ImGui::DragFloat2(prop.first.c_str(), static_cast<float*>(prop.second.value), prop.second.speed, prop.second.min, prop.second.max);
			break;
		case generated_mesh_param_type::FLOAT_4:
			changed |= ImGui::DragFloat4(prop.first.c_str(), static_cast<float*>(prop.second.value), prop.second.speed, prop.second.min, prop.second.max);
			break;
		case generated_mesh_param_type::FLOAT_4_SUM_1:
			{
				f32* const f = static_cast<f32*>(prop.second.value);
				const f32 of[4] = { f[0], f[1], f[2], f[3] };
				changed |= ImGui::DragFloat4(prop.first.c_str(), f, prop.second.speed, prop.second.min, prop.second.max);
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
	img_req["tiling"] = true;
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
	s32 cur_sec = (s32)std::floor(m_app_ctx->mgl_ctx.time.now);
	std::filesystem::path outp = outdirp / (std::to_string(cur_sec) + ".png");
	std::ofstream outf(outp, std::ios::out | std::ios::binary);
	outf << b64_decoded;
	outf.close();
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
