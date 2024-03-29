#include "pch.h"
#include "app_ctx.h"
#include "geom/generated_mesh.h"
#include "core/sgnode.h"
#include "core/scene_material.h"
#include "core/action.h"
#include "scene_graph_window.h"
#include "materials_list_window.h"
#include "core/smnode.h"

app_ctx::app_ctx() :
	mgl_ctx(1280, 720, "PowerTranzphormR", { .vsync = true, .clear = { .r = 1.0f, .g = 0.0f, .b = 1.0f } }),
	actions(&scene, this),
	preview_fb(1, 1280, 720)
{
	NFD_Init(); // Should happen after GLFW initialized
	g::init();
	scene.clear(); // Need to create the default material after initializing globals
	reset_camera();
	init_menus();

	constexpr u64 nverts = 6;
	constexpr f32 s = .1f;
	constexpr f32 verts[nverts * 6] = {
		0, s, 0, 0, 1, 0,	// 0 top
		0, 0, s, 0, 0, 1,	// 1 front
		s, 0, 0, 1, 0, 0,	// 2 right
		0, 0, -s, 0, 0, -1, // 3 back
		-s, 0, 0, -1, 0, 0, // 4 left
		0, -s, 0, 0, -1, 0, // 5 bottom
	};
	constexpr u64 nindices = 3 * 8;
	constexpr u32 indices[nindices] = {
		1, 2, 0, //
		2, 3, 0, //
		3, 4, 0, //
		4, 1, 0, //
		1, 5, 2, //
		2, 5, 3, //
		3, 5, 4, //
		4, 5, 1, //
	};
	m_vertex_editor_icon.icon.init(verts, nverts, { 3, 3 }, indices, nindices);
}
app_ctx::~app_ctx()
{
	scene.destroy();
	g::destroy();
}



void app_ctx::clear()
{
	loaded_filename = "";
	actions.clear();
	scene.clear();
	reset_camera();
	deselect_all();
	clear_clipboard();
	frozen2unfrozen.clear();
	NFD_Quit(); // Should happen before GLFW destroyed
}
bool app_ctx::save()
{
	if (loaded_filename.size())
	{
		save(loaded_filename);
		return true;
	}
	else
	{
		return save_as();
	}
}
void app_ctx::save(const std::string& fp)
{
	std::ofstream out(fp);
	assert(out.is_open());
	loaded_filename = fp;

	actions.save(out, scene.get_sg_root());

	nlohmann::json f2u;
	std::vector<nlohmann::json::array_t> pairs;
	for (const auto& pair : frozen2unfrozen)
	{
		if (pair.first != clipboard)
		{
			pairs.push_back({ pair.first->get_id(), pair.second->get_id() });
		}
	}
	f2u["p"] = pairs;
	f2u["n"] = pairs.size();
	out << f2u << "\n";

	scene.save(out, fp);
}
bool app_ctx::save_as()
{
	const std::string& fp = u::save_dialog(mgl_ctx.window, "PowerTranzphormR Scene", "phorm");
	if (!fp.empty())
	{
		save(fp);
		return true;
	}
	return false;
}
bool app_ctx::export_as()
{
	const std::string& fp = u::save_dialog(mgl_ctx.window, "PowerTranzphormR X-Port", "xport");
	if (!fp.empty())
	{
		haul::output_file out(fp);
		scene.save_xport(&out);
		return true;
	}
	return false;
}
bool app_ctx::confirm_unsaved_changes()
{
	const s32 res = u::confirm_message_box(mgl_ctx.window, L"Do you want to save your changes?", L"PowerTranzphormR");
	if (res == IDYES)
	{
		return save(); // Only continue if save was successful
	}
	else if (res == IDNO)
	{
		return true; // Continue (true) without saving
	}
	return false;
}
void app_ctx::load(const std::string& fp)
{
	clear();
	std::ifstream in(fp);
	assert(in.is_open());
	loaded_filename = fp;

	const auto& nodes = actions.load(in);

	const nlohmann::json& f2u = u::next_line_json(in);
	const nlohmann::json::array_t& f2us = f2u["p"];
	for (u64 i = 0; i < f2u["n"]; i++)
	{
		assert(nodes.contains(f2us[i][0]) && nodes.contains(f2us[i][1]));
		frozen2unfrozen.insert({ nodes.at(f2us[i][0]), nodes.at(f2us[i][1]) });
	}

	const std::string& rid = scene.load(in, fp);
	assert(nodes.contains(rid));
	scene.set_sg_root(nodes.at(rid));
	deselect_all();
}
void app_ctx::undo()
{
	m_multiselect_sgnodes.clear();
	const sgnode* const selected = get_selected_sgnode();
	const action* const a = actions.undo();
	if (a)
	{
		if (selected && a->undo_conflict(selected))
			set_selected_sgnode(nullptr);
	}
}
void app_ctx::redo()
{
	m_multiselect_sgnodes.clear();
	const sgnode* const selected = get_selected_sgnode();
	const action* const a = actions.redo();
	if (a)
	{
		if (selected && a->redo_conflict(selected))
			set_selected_sgnode(nullptr);
	}
}
bool app_ctx::has_unfrozen(const sgnode* const node) const
{
	return frozen2unfrozen.contains(node);
}
void app_ctx::set_sel_type(const global_selection_type t)
{
	if (sel_type != t)
	{
		sel_type = t;
		if (sel_type == global_selection_type::sgnode)
		{
			m_imgui_needs_select_unfocused_sgnode = m_selected_sgnode;
		}
		else if (sel_type == global_selection_type::material)
		{
			m_imgui_needs_select_unfocused_mtl = m_selected_mtl;
		}
		else if (sel_type == global_selection_type::light)
		{
			m_imgui_needs_select_unfocused_light = m_selected_light;
		}
		else if (sel_type == global_selection_type::waypoint)
		{
			m_imgui_needs_select_unfocused_waypoint = m_selected_waypoint;
		}
		else if (sel_type == global_selection_type::static_mesh)
		{
			m_imgui_needs_select_unfocused_static_mesh = m_selected_static_mesh;
		}
	}
}
void app_ctx::set_selected_sgnode(sgnode* const node)
{
	if (node)
	{
		set_sel_type(global_selection_type::sgnode);
	}
	if (m_selected_sgnode != node || m_multiselect_sgnodes.size() > 0)
	{
		m_multiselect_sgnodes.clear();
		m_selected_sgnode = node;
		m_imgui_needs_select_unfocused_sgnode = m_selected_sgnode;
	}
}
void app_ctx::toggle_sgnode_multiselected(sgnode* const node)
{
	if (m_multiselect_sgnodes.size() == 0)
	{
		sgnode* const prev_selected = get_selected_sgnode();
		if (prev_selected)
		{
			set_selected_sgnode(nullptr);
			m_multiselect_sgnodes.emplace_back(prev_selected);
		}
	}
	const auto& it = std::find(m_multiselect_sgnodes.begin(), m_multiselect_sgnodes.end(), node);
	if (it != m_multiselect_sgnodes.end())
	{
		m_multiselect_sgnodes.erase(it);
	}
	else
	{
		m_multiselect_sgnodes.emplace_back(node);
	}
}
size_t app_ctx::get_num_multiselected_sgnodes() const
{
	return m_multiselect_sgnodes.size();
}
bool app_ctx::is_sgnode_multiselected(sgnode* const node) const
{
	return std::find(m_multiselect_sgnodes.cbegin(), m_multiselect_sgnodes.cend(), node) != m_multiselect_sgnodes.cend();
}
sgnode* app_ctx::get_selected_sgnode()
{
	return (sel_type == global_selection_type::sgnode && m_multiselect_sgnodes.size() == 0) ? m_selected_sgnode : nullptr;
}
sgnode* app_ctx::get_imgui_needs_select_unfocused_sgnode()
{
	return m_imgui_needs_select_unfocused_sgnode;
}
void app_ctx::unset_imgui_needs_select_unfocused_sgnode()
{
	m_imgui_needs_select_unfocused_sgnode = nullptr;
}
void app_ctx::set_selected_material(scene_material* const mtl)
{
	if (mtl)
	{
		set_sel_type(global_selection_type::material);
	}
	if (m_selected_mtl != mtl)
	{
		m_selected_mtl = mtl;
		m_imgui_needs_select_unfocused_mtl = m_selected_mtl;
	}
}
scene_material* app_ctx::get_selected_material()
{
	return sel_type == global_selection_type::material ? m_selected_mtl : nullptr;
}
scene_material* app_ctx::get_imgui_needs_select_unfocused_mtl()
{
	return m_imgui_needs_select_unfocused_mtl;
}
void app_ctx::unset_imgui_needs_select_unfocused_mtl()
{
	m_imgui_needs_select_unfocused_mtl = nullptr;
}
void app_ctx::set_sg_window(scene_graph_window* const window)
{
	assert(!m_sg_window);
	m_sg_window = window;
}
void app_ctx::set_mtls_window(materials_list_window* const window)
{
	assert(!m_mtls_window);
	m_mtls_window = window;
}
std::vector<std::pair<u32, scene_material*>> app_ctx::get_sorted_materials()
{
	const auto& unordered_mtls = scene.get_materials();
	std::vector<std::pair<u32, scene_material*>> sorted_mtls;
	for (const auto& i : unordered_mtls)
	{
		if (i.first != 0)
		{
			sorted_mtls.emplace_back(std::make_pair(i.first, i.second));
		}
	}
	std::sort(sorted_mtls.begin(), sorted_mtls.end(), [](const auto& a, const auto& b)
		{
			return _strnicmp(a.second->get_name().c_str(), b.second->get_name().c_str(), std::max(a.second->get_name().size(), b.second->get_name().size())) < 0;
		});
	sorted_mtls.emplace(sorted_mtls.begin(), std::make_pair(0, unordered_mtls.at(0)));
	return sorted_mtls;
}
void app_ctx::remove_material(const u32 id)
{
	scene.erase_material(id);
	scene.get_sg_root()->replace_material(&scene, id, 0);
	for (const auto& pair : frozen2unfrozen)
	{
		pair.second->replace_material(&scene, id, 0);
	}
	if (clipboard)
		clipboard->replace_material(&scene, id, 0);
}
void app_ctx::set_material(sgnode* const node, const u32 id)
{
	node->set_material(&scene, id);
	const auto& it = frozen2unfrozen.find(node);
	if (it != frozen2unfrozen.end())
	{
		if (!it->second->is_operation())
			it->second->set_material(&scene, id);
	}
}
void app_ctx::add_light()
{
	set_selected_light(scene.add_light());
}
void app_ctx::set_selected_light(light* const l)
{
	if (l)
	{
		set_sel_type(global_selection_type::light);
	}
	if (m_selected_light != l)
	{
		m_selected_light = l;
		m_imgui_needs_select_unfocused_light = m_selected_light;
	}
}
light* app_ctx::get_selected_light()
{
	return sel_type == global_selection_type::light ? m_selected_light : nullptr;
}
void app_ctx::unset_imgui_needs_select_unfocused_light()
{
	m_imgui_needs_select_unfocused_light = nullptr;
}
light* app_ctx::get_imgui_needs_select_unfocused_light()
{
	return m_imgui_needs_select_unfocused_light;
}
void app_ctx::add_waypoint()
{
	set_selected_waypoint(scene.add_waypoint());
}
void app_ctx::set_selected_waypoint(waypoint* const w)
{
	if (w)
	{
		set_sel_type(global_selection_type::waypoint);
	}
	if (m_selected_waypoint != w)
	{
		m_selected_waypoint = w;
		m_imgui_needs_select_unfocused_waypoint = m_selected_waypoint;
	}
}
waypoint* app_ctx::get_selected_waypoint()
{
	return sel_type == global_selection_type::waypoint ? m_selected_waypoint : nullptr;
}
void app_ctx::unset_imgui_needs_select_unfocused_waypoint()
{
	m_imgui_needs_select_unfocused_waypoint = nullptr;
}
waypoint* app_ctx::get_imgui_needs_select_unfocused_waypoint()
{
	return m_imgui_needs_select_unfocused_waypoint;
}
void app_ctx::draw_vertex_editor_icon()
{
	glDisable(GL_CULL_FACE);
	glDepthMask(GL_FALSE);
	static f32 rot_x = 0.f, rot_y = 0.f, rot_z = 0.f;
	if (m_vertex_editor_icon.show)
	{
		const auto& vei_mat = m_vertex_editor_icon.transform * tmat_util::rotation_yxz<space::OBJECT>(rot_x, rot_y, rot_z);
		const tmat<space::OBJECT, space::WORLD> normal = vei_mat.invert_copy().transpose_copy();
		const mat<space::OBJECT, space::CLIP>& mvp = preview_cam.get_view_proj() * vei_mat;
		m_vertex_editor_icon.shaders.uniform_mat4("u_mvp", mvp.e);
		m_vertex_editor_icon.shaders.uniform_mat4("u_model", vei_mat.e);
		m_vertex_editor_icon.shaders.uniform_mat4("u_normal", normal.e);
		m_vertex_editor_icon.shaders.uniform_3fv("u_cam_pos", preview_cam.get_pos().e);
		m_vertex_editor_icon.shaders.uniform_3fv("u_cam_dir", preview_cam.get_view().k);
		m_vertex_editor_icon.shaders.uniform_1f("u_time", mgl_ctx.time.now);
		m_vertex_editor_icon.shaders.uniform_1f("u_switch_time", m_vertex_editor_icon.switch_time);
		mgl_ctx.draw(m_vertex_editor_icon.icon, m_vertex_editor_icon.shaders);
		m_vertex_editor_icon.show = false;
		const f32 dt = mgl_ctx.time.now - m_vertex_editor_icon.switch_time;
		const f32 base_speed = .02f;
		const f32 e = expf(-dt * 5);
		const f32 xz_coeff = .1f * (dt > .5f);
		rot_x += (base_speed + e * .05f) * xz_coeff;
		rot_y += base_speed + e * .225f;
		rot_z += (base_speed + e * .05f) * xz_coeff;
	}
	else
	{
		rot_x = 0.f;
		rot_z = 0.f;
		m_vertex_editor_icon.cur_selected_vtx = -1;
		m_vertex_editor_icon.prev_selected_vtx = -1;
	}
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
}
void app_ctx::set_vertex_editor_icon_position(const point<space::WORLD>& p, const s32 cur_idx)
{
	m_vertex_editor_icon.cur_selected_vtx = cur_idx;
	m_vertex_editor_icon.transform = tmat_util::translation<space::OBJECT, space::WORLD>(p);
	m_vertex_editor_icon.show = true;
}
void app_ctx::check_vertex_editor_icon_switched()
{
	if (m_vertex_editor_icon.cur_selected_vtx != m_vertex_editor_icon.prev_selected_vtx)
	{
		m_vertex_editor_icon.switch_time = mgl_ctx.time.now;
		m_vertex_editor_icon.prev_selected_vtx = m_vertex_editor_icon.cur_selected_vtx;
	}
}
void app_ctx::set_selected_static_mesh(smnode* const m)
{
	if (m)
	{
		set_sel_type(global_selection_type::static_mesh);
	}
	if (m_selected_static_mesh != m)
	{
		m_vertex_editor_icon.cur_selected_vtx = -1;
		m_vertex_editor_icon.prev_selected_vtx = -1;
		m_selected_static_mesh = m;
		m_imgui_needs_select_unfocused_static_mesh = m_selected_static_mesh;
	}
}
smnode* app_ctx::get_imgui_needs_select_unfocused_static_mesh()
{
	return m_imgui_needs_select_unfocused_static_mesh;
}
void app_ctx::unset_imgui_needs_select_unfocused_static_mesh()
{
	m_imgui_needs_select_unfocused_static_mesh = nullptr;
}
smnode* app_ctx::get_selected_static_mesh()
{
	return sel_type == global_selection_type::static_mesh ? m_selected_static_mesh : nullptr;
}
void app_ctx::deselect_all()
{
	set_selected_sgnode(nullptr);
	set_selected_material(nullptr);
	set_selected_light(nullptr);
	set_selected_waypoint(nullptr);
	set_selected_static_mesh(nullptr);
	set_sel_type(global_selection_type::sgnode);
}
void app_ctx::clear_clipboard()
{
	if (clipboard)
	{
		if (clipboard->is_frozen() && frozen2unfrozen.contains(clipboard))
		{
			sgnode* const unfrozen = frozen2unfrozen.at(clipboard);
			delete unfrozen;
			frozen2unfrozen.erase(clipboard);
		}
		delete clipboard;
		clipboard = nullptr;
	}
}
void app_ctx::destroy_static_mesh(smnode* const n)
{
	if (n == m_selected_static_mesh)
	{
		deselect_all();
		unset_imgui_needs_select_unfocused_static_mesh();
	}
	scene.destroy_static_mesh(n);
}
void app_ctx::destroy_light(light* const l)
{
	if (l == m_selected_light)
	{
		deselect_all();
		unset_imgui_needs_select_unfocused_light();
	}
	scene.destroy_light(l);
}
void app_ctx::destroy_waypoint(waypoint* const w)
{
	if (w == m_selected_waypoint)
	{
		deselect_all();
		unset_imgui_needs_select_unfocused_waypoint();
	}
	scene.destroy_waypoint(w);
}
void app_ctx::make_sgnode_static(const sgnode* const node)
{
	const sgnode* const parent = node->get_parent();
	assert(parent);

	// carve -> gluTess
	std::unordered_map<u32, std::vector<mesh_vertex>> tesselate_verts_for_mtl;
	const auto& all_mtls = scene.get_materials();
	for (auto it = all_mtls.begin(); it != all_mtls.end(); ++it)
	{
		tesselate_verts_for_mtl.insert(std::make_pair(it->first, std::vector<mesh_vertex>()));
	}
	scene.tesselate_external(node->get_gen()->mesh, tesselate_verts_for_mtl);

	// deduplicate vertices by position
	std::unordered_map<std::string, vertex_t*> verts_map;
	for (const auto& pair : tesselate_verts_for_mtl)
	{
		for (const auto& v : pair.second)
		{
			const std::string& k = v.hash_pos();
			if (!verts_map.contains(k))
			{
				verts_map.insert({ k, new vertex_t(carve::geom::VECTOR(v.x, v.y, v.z)) });
			}
		}
	}

	// gluTess -> triangulated carve
	std::vector<face_t*> carve_faces;
	for (const auto& pair : tesselate_verts_for_mtl)
	{
		if (pair.second.size() == 0)
		{
			continue;
		}

		auto& mtl_id_attr = scene.get_mtl_id_attr();
		auto& vert_attrs = scene.get_vert_attrs();
		for (size_t i = 0; i < pair.second.size(); i += 3)
		{
			const u32 mtl_id = pair.first;

			vertex_t* va = verts_map.at(pair.second[i].hash_pos());
			vertex_t* vb = verts_map.at(pair.second[i + 1].hash_pos());
			vertex_t* vc = verts_map.at(pair.second[i + 2].hash_pos());
			face_t* const new_f = new face_t(va, vb, vc);
			mtl_id_attr.setAttribute(new_f, mtl_id);
			for (u32 j = 0; j < 3; ++j)
			{
				const auto& srcv = pair.second[i + j];
				const tex_coord_t uv0(srcv.u0, srcv.v0, srcv.uo0, srcv.vo0);
				const tex_coord_t uv1(srcv.u1, srcv.v1, srcv.uo1, srcv.vo1);
				const tex_coord_t uv2(srcv.u2, srcv.v2, srcv.uo2, srcv.vo2);
				const tex_coord_t uv3(srcv.u3, srcv.v3, srcv.uo3, srcv.vo3);
				const color_t color(srcv.r, srcv.g, srcv.b, srcv.a);
				vert_attrs.uv0.setAttribute(new_f, j, uv0);
				vert_attrs.uv1.setAttribute(new_f, j, uv1);
				vert_attrs.uv2.setAttribute(new_f, j, uv2);
				vert_attrs.uv3.setAttribute(new_f, j, uv3);
				vert_attrs.w0.setAttribute(new_f, j, srcv.w0);
				vert_attrs.w1.setAttribute(new_f, j, srcv.w1);
				vert_attrs.w2.setAttribute(new_f, j, srcv.w2);
				vert_attrs.w3.setAttribute(new_f, j, srcv.w3);
				vert_attrs.color.setAttribute(new_f, j, color);
			}
			carve_faces.push_back(new_f);
		}
	}
	mesh_t* const new_carve_mesh = new mesh_t(carve_faces);

	// triangulated carve -> smnode
	generated_static_mesh* const new_gen = new generated_static_mesh(new_carve_mesh, &scene);
	smnode* const new_sm = new smnode(new_gen, node->accumulate_mats(), "Static " + node->get_name());
	new_sm->copy_properties_from(*node);
	new_sm->set_snap_angle(scene_ctx::s_snap_angle);
	new_sm->set_should_snap_all(scene_ctx::s_snap_all);
	new_sm->set_should_snap(true);
	scene.add_static_mesh(new_sm);
	set_selected_static_mesh(new_sm);

	for (const auto& pair : verts_map)
	{
		delete pair.second;
	}
}
void app_ctx::make_frozen_sgnode_from_smnode(const smnode* const node)
{
	generated_static_mesh* const new_gen = new generated_static_mesh(carve_clone(node->get_mesh(), &scene), &scene);
	sgnode* const root = scene.get_sg_root();
	const tmat<space::WORLD, space::OBJECT>& root_inv = root->accumulate_mats().invert_copy();
	const tmat<space::OBJECT, space::OBJECT>& inv = root_inv * node->get_mat();
	sgnode* const new_node = new sgnode(nullptr, new_gen, "Phrozen " + node->get_name(), inv);
	new_node->copy_properties_from(*node);
	create_action(new_node, root);
	set_selected_sgnode(new_node);
}
void app_ctx::group_to_operation_action(const carve::csg::CSG::OP op_type)
{
	const u32 skip_count = (u32)m_multiselect_sgnodes.size();
	sgnode* const op = new sgnode(nullptr, op_type);
	actions.create(op, scene.get_sg_root(), skip_count);
	for (sgnode* const node : m_multiselect_sgnodes)
	{
		actions.reparent(node, op, -1, skip_count);
	}
	m_multiselect_sgnodes.clear();
	set_selected_sgnode(op);
}

void app_ctx::transform_action(sgnode* const t, const tmat<space::OBJECT, space::PARENT>& old_mat, const tmat<space::OBJECT, space::PARENT>& new_mat)
{
	actions.transform(t, old_mat, new_mat);
}
void app_ctx::reparent_action(sgnode* const target, sgnode* const new_parent, const s64 new_index)
{
	actions.reparent(target, new_parent, new_index);
}
void app_ctx::create_action(sgnode* const target, sgnode* const parent, const u32 skip_count)
{
	actions.create(target, parent, skip_count);
}
void app_ctx::destroy_action(sgnode* const target)
{
	actions.destroy(target);
}
void app_ctx::destroy_selected_action()
{
	sgnode* const selected = get_selected_sgnode();
	assert(selected && selected->get_parent());
	actions.destroy(selected);
	set_selected_sgnode(nullptr);
}
void app_ctx::create_cube_action()
{
	create_shape_action(&scene_ctx::generated_textured_cuboid, "Cube");
}
void app_ctx::create_sphere_action()
{
	create_shape_action(&scene_ctx::generated_textured_ellipsoid, "Sphere");
}
void app_ctx::create_cylinder_action()
{
	create_shape_action(&scene_ctx::generated_textured_cylinder, "Cylinder");
}
void app_ctx::create_cone_action()
{
	create_shape_action(&scene_ctx::generated_textured_cone, "Cone");
}
void app_ctx::create_torus_action()
{
	create_shape_action(&scene_ctx::generated_textured_torus, "Torus");
}
void app_ctx::create_heightmap()
{
	set_selected_static_mesh(scene.add_static_mesh());
}
void app_ctx::create_union_action()
{
	create_operation_action(carve::csg::CSG::OP::UNION);
}
void app_ctx::create_subtract_action()
{
	create_operation_action(carve::csg::CSG::OP::A_MINUS_B);
}
void app_ctx::create_intersect_action()
{
	create_operation_action(carve::csg::CSG::OP::INTERSECTION);
}
void app_ctx::freeze_action(sgnode* const target)
{
	assert(!target->is_root());
	sgnode* const new_node = actions.freeze(target);
	frozen2unfrozen.insert({ new_node, target });
}
void app_ctx::unfreeze_action(sgnode* const target)
{
	const auto& it = frozen2unfrozen.find(target);
	assert(it != frozen2unfrozen.end());
	actions.unfreeze(target, it->second);
	frozen2unfrozen.erase(it);
}
void app_ctx::rename_action(sgnode* const target, const std::string& new_name)
{
	actions.rename(target, new_name);
}
void app_ctx::set_operation_action(sgnode* const target, const carve::csg::CSG::OP new_op)
{
	actions.set_operation(target, new_op);
}
void app_ctx::duplicate_selected_static_mesh()
{
	smnode* const selected = get_selected_static_mesh();
	smnode* const cloned = selected->clone(&scene);
	scene.add_static_mesh(cloned);
	set_selected_static_mesh(cloned);
}
void app_ctx::duplicate_selected_light()
{
	light* const selected = get_selected_light();
	light* const cloned = selected->clone();
	scene.add_light(cloned);
	set_selected_light(cloned);
}
void app_ctx::duplicate_selected_waypoint()
{
	waypoint* const selected = get_selected_waypoint();
	waypoint* const cloned = selected->clone();
	scene.add_waypoint(cloned);
	set_selected_waypoint(cloned);
}
void app_ctx::duplicate_selected_material()
{
	scene_material* const selected = get_selected_material();
	scene_material* const cloned = selected->clone();
	scene.add_material(cloned);
	set_selected_material(cloned);
}



void app_ctx::reset_camera()
{
	f32 ar = static_cast<f32>(preview_fb.get_width()) / static_cast<f32>(preview_fb.get_height());
	preview_cam = mgl::camera({ 0, 0, 5 }, 0, 0, 108 / ar, ar, 0.1f, 1000.0f, 5.0f);
}
void app_ctx::create_operation_action(const carve::csg::CSG::OP op)
{
	create(new sgnode(nullptr, op));
}
template<typename FN>
void app_ctx::create_shape_action(FN fn, const std::string& name)
{
	generated_mesh* gen = (scene.*fn)(0, {});
	create(new sgnode(nullptr, gen, name));
}
void app_ctx::create(sgnode* const node)
{
	sgnode* const selected = get_selected_sgnode();
	assert(selected);
	if (selected->is_operation())
	{
		actions.create(node, selected);
	}
	else
	{
		assert(!selected->is_root());
		actions.create(node, selected->get_parent());
	}
	set_selected_sgnode(node);
}
void app_ctx::init_menus()
{
	file_menu();
	phorm_menu();
	material_menu();
	static_meshes_menu();
	lights_menu();
	waypoints_menu();
}
void app_ctx::file_menu()
{
	shortcut_menu file_menu;
	file_menu.name = "File";
	shortcut_menu_item file_new = {
		"New",
		[&]()
		{
			if (!confirm_unsaved_changes())
			{
				return;
			}
			clear();
		},
		[]()
		{
			return true;
		},
		"Ctrl+N",
		GLFW_KEY_N,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item file_open = {
		"Open",
		[&]()
		{
			if (!confirm_unsaved_changes())
			{
				return;
			}
			const std::string& fp = u::open_dialog(mgl_ctx.window, "PowerTranzphormR Scene", "phorm");
			if (!fp.empty())
				load(fp);
		},
		[]()
		{
			return true;
		},
		"Ctrl+O",
		GLFW_KEY_O,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item file_save = {
		"Save",
		[&]()
		{
			save();
		},
		[]()
		{
			return true;
		},
		"Ctrl+S",
		GLFW_KEY_S,
		GLFW_MOD_CONTROL,
	};
	/*
	shortcut_menu_item file_save_as = {
		"Save As...",
		[&]()
		{
			save_as();
		},
		[]()
		{
			return true;
		},
		"Ctrl+Shift+S",
		GLFW_KEY_S,
		GLFW_MOD_CONTROL | GLFW_MOD_SHIFT,
	};
	*/

	shortcut_menu_item file_export = {
		"Export...",
		[&]()
		{
			export_as();
		},
		[]()
		{
			return true;
		},
		"",
		0,
		0,
	};

	file_menu.groups.push_back({ file_new, file_open, file_save, file_export /* , file_save_as , file_export */ });
	shortcut_menus.push_back(file_menu);
}
void app_ctx::phorm_menu()
{
	shortcut_menu_item phorm_create_cube = {
		"Cube",
		[&]()
		{
			create_cube_action();
		},
		[&]()
		{
			return true;
		},
		"Ctrl+1",
		GLFW_KEY_1,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_create_sphere = {
		"Sphere",
		[&]()
		{
			create_sphere_action();
		},
		[&]()
		{
			return true;
		},
		"Ctrl+2",
		GLFW_KEY_2,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_create_cylinder = {
		"Cylinder",
		[&]()
		{
			create_cylinder_action();
		},
		[&]()
		{
			return true;
		},
		"Ctrl+3",
		GLFW_KEY_3,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_create_cone = {
		"Cone",
		[&]()
		{
			create_cone_action();
		},
		[&]()
		{
			return true;
		},
		"Ctrl+4",
		GLFW_KEY_4,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_create_torus = {
		"Torus",
		[&]()
		{
			create_torus_action();
		},
		[&]()
		{
			return true;
		},
		"Ctrl+5",
		GLFW_KEY_5,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_create_union = {
		"Union",
		[&]()
		{
			create_union_action();
		},
		[&]()
		{
			return true;
		},
		"Ctrl+=",
		GLFW_KEY_EQUAL,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_create_subtract = {
		"Subtract",
		[&]()
		{
			create_subtract_action();
		},
		[&]()
		{
			return true;
		},
		"Ctrl+-",
		GLFW_KEY_MINUS,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_create_intersect = {
		"Intersect",
		[&]()
		{
			create_intersect_action();
		},
		[&]()
		{
			return true;
		},
		"Ctrl+8",
		GLFW_KEY_8,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_create = {
		"Create",
		[&]() {},
		[&]()
		{
			return get_selected_sgnode();
		},
		"",
		0,
		0,
		{
			{ phorm_create_cube, phorm_create_sphere, phorm_create_cylinder, phorm_create_cone, phorm_create_torus },
			{ phorm_create_union, phorm_create_subtract, phorm_create_intersect },
		}
	};

	shortcut_menu_item phorm_undo = {
		"Undo",
		[&]()
		{
			undo();
		},
		[&]()
		{
			return actions.can_undo();
		},
		"Ctrl+Z",
		GLFW_KEY_Z,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_redo = {
		"Redo",
		[&]()
		{
			redo();
		},
		[&]()
		{
			return actions.can_redo();
		},
		"Ctrl+Y",
		GLFW_KEY_Y,
		GLFW_MOD_CONTROL,
	};

	shortcut_menu_item phorm_cut = {
		"Cut",
		[&]()
		{
			clear_clipboard();
			sgnode* const selected = get_selected_sgnode();
			clipboard = selected->clone(this);
			if (selected->is_frozen() && frozen2unfrozen.contains(selected))
			{
				// transfer ownership of unfrozen node to new clone
				sgnode* const unfrozen = frozen2unfrozen.at(selected);
				frozen2unfrozen.erase(selected);
				frozen2unfrozen.insert({ clipboard, unfrozen });
			}
			destroy_action(selected);
		},
		[&]()
		{
			sgnode* const selected = get_selected_sgnode();
			return selected && !selected->is_root();
		},
		"Ctrl+X",
		GLFW_KEY_X,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_copy = {
		"Copy",
		[&]()
		{
			clear_clipboard();
			sgnode* const selected = get_selected_sgnode();
			clipboard = selected->clone(this);
			if (selected->is_frozen() && frozen2unfrozen.contains(selected))
			{
				sgnode* const unfrozen = frozen2unfrozen.at(selected);
				sgnode* const unfrozen_clone = unfrozen->clone(this);
				frozen2unfrozen.insert({ clipboard, unfrozen_clone });
			}
		},
		[&]()
		{
			return get_selected_sgnode();
		},
		"Ctrl+C",
		GLFW_KEY_C,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_paste = {
		"Paste",
		[&]()
		{
			sgnode* const selected = get_selected_sgnode();
			sgnode* clone = nullptr;
			if (selected->is_operation())
			{
				clone = clipboard->clone_self_and_insert(this, selected);
			}
			else
			{
				assert(selected->get_parent());
				clone = clipboard->clone_self_and_insert(this, selected->get_parent());
			}
			set_selected_sgnode(clone);

			if (clipboard->is_frozen() && frozen2unfrozen.contains(clipboard))
			{
				sgnode* const unfrozen = frozen2unfrozen.at(clipboard);
				sgnode* const unfrozen_clone = unfrozen->clone_self_and_insert(this, nullptr);
				frozen2unfrozen.insert({ clone, unfrozen_clone });
			}
		},
		[&]()
		{
			sgnode* const selected = get_selected_sgnode();
			return clipboard && selected;
		},
		"Ctrl+V",
		GLFW_KEY_V,
		GLFW_MOD_CONTROL,
	};

	shortcut_menu_item phorm_move_up = {
		"Move Up",
		[&]()
		{
			sgnode* const selected = get_selected_sgnode();
			const s64 i = selected->get_index();
			reparent_action(selected, selected->get_parent(), i - 1);
		},
		[&]()
		{
			const sgnode* const selected = get_selected_sgnode();
			return selected && !selected->is_root() && selected->get_index() > 0;
		},
		"Ctrl+Up",
		GLFW_KEY_UP,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_move_down = {
		"Move Down",
		[&]()
		{
			sgnode* const selected = get_selected_sgnode();
			assert(selected);
			const s64 i = selected->get_index();
			reparent_action(selected, selected->get_parent(), i + 1);
		},
		[&]()
		{
			const sgnode* const selected = get_selected_sgnode();
			return selected && !selected->is_root() && selected->get_index() < static_cast<s64>(selected->get_parent()->get_children().size() - 1);
		},
		"Ctrl+Down",
		GLFW_KEY_DOWN,
		GLFW_MOD_CONTROL,
	};

	shortcut_menu_item phorm_hide = {
		"Hide",
		[&]()
		{
			get_selected_sgnode()->set_visibility(false);
		},
		[&]()
		{
			const sgnode* const selected = get_selected_sgnode();
			return selected && !selected->is_root() && selected->is_visible();
		},
		"Ctrl+H",
		GLFW_KEY_H,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_show = {
		"Show",
		[&]()
		{
			get_selected_sgnode()->set_visibility(true);
		},
		[&]()
		{
			const sgnode* const selected = get_selected_sgnode();
			return selected && !selected->is_root() && !selected->is_visible();
		},
		"Ctrl+H",
		GLFW_KEY_H,
		GLFW_MOD_CONTROL,
	};

	shortcut_menu_item phorm_group_to_union = {
		"Union",
		[&]()
		{
			group_to_operation_action(carve::csg::CSG::OP::UNION);
		},
		[&]()
		{
			return true;
		},
		"Ctrl+=",
		GLFW_KEY_EQUAL,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_group_to_subtract = {
		"Subtract",
		[&]()
		{
			group_to_operation_action(carve::csg::CSG::OP::A_MINUS_B);
		},
		[&]()
		{
			return true;
		},
		"Ctrl+-",
		GLFW_KEY_MINUS,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_group_to_intersect = {
		"Intersect",
		[&]()
		{
			group_to_operation_action(carve::csg::CSG::OP::INTERSECTION);
		},
		[&]()
		{
			return true;
		},
		"Ctrl+8",
		GLFW_KEY_8,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_group = {
		"Group to",
		[&]() {},
		[&]()
		{
			return m_multiselect_sgnodes.size() > 0;
		},
		"",
		0,
		0,
		{
			{ phorm_group_to_union, phorm_group_to_subtract, phorm_group_to_intersect },
		}
	};

	shortcut_menu_item phorm_phreeze = {
		"Phreeze!",
		[&]()
		{
			freeze_action(get_selected_sgnode());
		},
		[&]()
		{
			const sgnode* const node = get_selected_sgnode();
			return node && !node->is_root() && node->get_gen()->mesh && !node->is_frozen();
		},
		"Ctrl+P",
		GLFW_KEY_P,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_unphreeze = {
		"Unphreeze!",
		[&]()
		{
			unfreeze_action(get_selected_sgnode());
		},
		[&]()
		{
			const sgnode* const node = get_selected_sgnode();
			return node && !node->is_root() && node->get_gen()->mesh && has_unfrozen(node);
		},
		"Ctrl+P",
		GLFW_KEY_P,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_clone_to_static = {
		"Clone to Static Mesh",
		[&]()
		{
			make_sgnode_static(get_selected_sgnode());
		},
		[&]()
		{
			const sgnode* const node = get_selected_sgnode();
			return node && !node->is_root() && node->is_frozen();
		},
		"Ctrl+Shift+P",
		GLFW_KEY_P,
		GLFW_MOD_CONTROL | GLFW_MOD_SHIFT,
	};

	shortcut_menu_item phorm_rename = {
		"Rename",
		[&]()
		{
			assert(m_sg_window);
			assert(!m_sg_window->get_renaming());
			m_sg_window->set_renaming(get_selected_sgnode());
		},
		[&]()
		{
			return get_selected_sgnode();
		},
		"Ctrl+R",
		GLFW_KEY_R,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_destroy = {
		"Destroy",
		[&]()
		{
			if (get_selected_sgnode())
			{
				if (!mgl_ctx.is_cursor_locked())
				{
					destroy_selected_action();
				}
			}
			else
			{
				assert(get_selected_static_mesh());
				destroy_static_mesh(get_selected_static_mesh());
			}
		},
		[&]()
		{
			sgnode* const sg = get_selected_sgnode();
			smnode* const sm = get_selected_static_mesh();
			return (sg && !sg->is_root()) || sm;
			// return selected && !selected->is_root();
		},
		"Delete",
		GLFW_KEY_DELETE,
		0,
	};

	shortcut_menu_item gizmodes = {
		"Gizmodes",
		[]() {
		},
		[&]()
		{
			return !mgl_ctx.is_cursor_locked();
		},
		"",
		0,
		0,
	};
	shortcut_menu_item gizmo_translate = {
		"Translate",
		[&]()
		{
			gizmo_op = ImGuizmo::OPERATION::TRANSLATE;
		},
		[&]()
		{
			return true;
		},
		"T",
		GLFW_KEY_T,
		0,
	};
	shortcut_menu_item gizmo_rotate = {
		"Rotate",
		[&]()
		{
			gizmo_op = ImGuizmo::OPERATION::ROTATE;
		},
		[&]()
		{
			return true;
		},
		"R",
		GLFW_KEY_R,
		0,
	};
	/*
	shortcut_menu_item gizmo_scale = {
		"Scale",
		[&]()
		{
			gizmo_op = ImGuizmo::OPERATION::SCALE;
		},
		[&]()
		{
			return true;
		},
		"E",
		GLFW_KEY_E,
		0,
	};
	*/
	gizmodes.groups.push_back({ gizmo_translate, gizmo_rotate /* , gizmo_scale */ });

	shortcut_menu phorm_menu;
	phorm_menu.name = "Phorm";
	phorm_menu.groups.push_back({ phorm_create, phorm_group });
	phorm_menu.groups.push_back({ phorm_undo, phorm_redo });
	phorm_menu.groups.push_back({ phorm_cut, phorm_copy, phorm_paste });
	phorm_menu.groups.push_back({ phorm_move_up, phorm_move_down });
	phorm_menu.groups.push_back({ phorm_hide, phorm_show });
	phorm_menu.groups.push_back({ phorm_phreeze, phorm_unphreeze, phorm_clone_to_static });
	phorm_menu.groups.push_back({ phorm_rename, phorm_destroy });
	phorm_menu.groups.push_back({ gizmodes });
	shortcut_menus.push_back(phorm_menu);
}
void app_ctx::material_menu()
{
	shortcut_menu_item material_create = {
		"Create",
		[&]()
		{
			scene_material* const mtl = scene.create_default_material();
			scene.add_material(mtl);
			set_selected_material(mtl);
		},
		[&]()
		{
			return true;
		},
		"Ctrl+Shift+=",
		GLFW_KEY_EQUAL,
		GLFW_MOD_CONTROL | GLFW_MOD_SHIFT,
	};
	shortcut_menu_item material_duplicate = {
		"Duplicate",
		[&]()
		{
			duplicate_selected_material();
		},
		[&]()
		{
			return get_selected_material();
		},
		"Ctrl+D",
		GLFW_KEY_D,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item material_rename = {
		"Rename",
		[&]()
		{
			assert(m_mtls_window);
			assert(!m_mtls_window->get_renaming());
			m_mtls_window->set_renaming(get_selected_material());
		},
		[&]()
		{
			return get_selected_material();
		},
		"Ctrl+R",
		GLFW_KEY_R,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item material_destroy = {
		"Destroy",
		[&]()
		{
			scene_material* const mtl = get_selected_material();
			remove_material(scene.get_id_for_material(mtl));
			set_selected_material(nullptr);
		},
		[&]()
		{
			return get_selected_material();
		},
		"Delete",
		GLFW_KEY_DELETE,
		0,
	};

	shortcut_menu material_menu;
	material_menu.name = "Material";
	material_menu.groups.push_back({ material_create });
	material_menu.groups.push_back({ material_duplicate });
	material_menu.groups.push_back({ material_rename, material_destroy });
	shortcut_menus.push_back(material_menu);
}
void app_ctx::static_meshes_menu()
{
	shortcut_menu_item sm_create = {
		"Create Heightmap",
		[&]()
		{
			create_heightmap();
		},
		[&]()
		{
			return true;
		},
		"Ctrl+Shift+H",
		GLFW_KEY_H,
		GLFW_MOD_CONTROL | GLFW_MOD_SHIFT,
	};
	shortcut_menu_item sm_duplicate = {
		"Duplicate",
		[&]()
		{
			duplicate_selected_static_mesh();
		},
		[&]()
		{
			return get_selected_static_mesh();
		},
		"Ctrl+D",
		GLFW_KEY_D,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item sm_rename = {
		"Rename",
		[&]()
		{
			assert(m_sg_window);
			assert(!m_sg_window->get_renaming_sm());
			m_sg_window->set_renaming_sm(get_selected_static_mesh());
		},
		[&]()
		{
			return get_selected_static_mesh();
		},
		"Ctrl+R",
		GLFW_KEY_R,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item sm_destroy = {
		"Destroy",
		[&]()
		{
			smnode* const sm = get_selected_static_mesh();
			destroy_static_mesh(sm);
			set_selected_static_mesh(nullptr);
		},
		[&]()
		{
			return get_selected_static_mesh();
		},
		"Delete",
		GLFW_KEY_DELETE,
		0,
	};

	shortcut_menu_item sm_hide = {
		"Hide",
		[&]()
		{
			get_selected_static_mesh()->set_visibility(false);
		},
		[&]()
		{
			const smnode* const selected = get_selected_static_mesh();
			return selected && selected->is_visible();
		},
		"Ctrl+H",
		GLFW_KEY_H,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item sm_show = {
		"Show",
		[&]()
		{
			get_selected_static_mesh()->set_visibility(true);
		},
		[&]()
		{
			const smnode* const selected = get_selected_static_mesh();
			return selected && !selected->is_visible();
		},
		"Ctrl+H",
		GLFW_KEY_H,
		GLFW_MOD_CONTROL,
	};

	shortcut_menu_item sm_clone_to_sg = {
		"Clone to Scene Graph",
		[&]()
		{
			make_frozen_sgnode_from_smnode(get_selected_static_mesh());
		},
		[&]()
		{
			return get_selected_static_mesh();
		},
		"Ctrl+Shift+P",
		GLFW_KEY_P,
		GLFW_MOD_CONTROL | GLFW_MOD_SHIFT,
	};

	shortcut_menu sm_menu;
	sm_menu.name = "Static Mesh";
	sm_menu.groups.push_back({ sm_create });
	sm_menu.groups.push_back({ sm_duplicate });
	sm_menu.groups.push_back({ sm_rename, sm_destroy });
	sm_menu.groups.push_back({ sm_hide, sm_show });
	sm_menu.groups.push_back({ sm_clone_to_sg });
	shortcut_menus.push_back(sm_menu);
}
void app_ctx::lights_menu()
{
	shortcut_menu_item light_create = {
		"Create",
		[&]()
		{
			add_light();
		},
		[&]()
		{
			return true;
		},
		"Ctrl+Shift+L",
		GLFW_KEY_L,
		GLFW_MOD_CONTROL | GLFW_MOD_SHIFT,
	};
	shortcut_menu_item light_duplicate = {
		"Duplicate",
		[&]()
		{
			duplicate_selected_light();
		},
		[&]()
		{
			const bool is_camera_light = (get_selected_light() == scene.get_lights()[0]);
			return get_selected_light() && !is_camera_light;
		},
		"Ctrl+D",
		GLFW_KEY_D,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item light_rename = {
		"Rename",
		[&]()
		{
			assert(m_sg_window);
			assert(!m_sg_window->get_renaming_light());
			m_sg_window->set_renaming_light(get_selected_light());
		},
		[&]()
		{
			const bool is_camera_light = (get_selected_light() == scene.get_lights()[0]);
			return get_selected_light() && !is_camera_light;
		},
		"Ctrl+R",
		GLFW_KEY_R,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item light_destroy = {
		"Destroy",
		[&]()
		{
			light* const l = get_selected_light();
			destroy_light(l);
			set_selected_light(nullptr);
		},
		[&]()
		{
			const bool is_camera_light = (get_selected_light() == scene.get_lights()[0]);
			return get_selected_light() && !is_camera_light;
		},
		"Delete",
		GLFW_KEY_DELETE,
		0,
	};

	shortcut_menu_item light_hide = {
		"Hide",
		[&]()
		{
			light* const selected = get_selected_light();
			selected->set_visibility(false);
			scene.update_light(selected);
		},
		[&]()
		{
			const light* const selected = get_selected_light();
			return selected && selected->is_visible();
		},
		"Ctrl+H",
		GLFW_KEY_H,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item light_show = {
		"Show",
		[&]()
		{
			light* const selected = get_selected_light();
			selected->set_visibility(true);
			scene.update_light(selected);
		},
		[&]()
		{
			const light* const selected = get_selected_light();
			return selected && !selected->is_visible();
		},
		"Ctrl+H",
		GLFW_KEY_H,
		GLFW_MOD_CONTROL,
	};

	shortcut_menu light_menu;
	light_menu.name = "Lights";
	light_menu.groups.push_back({ light_create });
	light_menu.groups.push_back({ light_duplicate });
	light_menu.groups.push_back({ light_rename, light_destroy });
	light_menu.groups.push_back({ light_hide, light_show });
	shortcut_menus.push_back(light_menu);
}
void app_ctx::waypoints_menu()
{
	shortcut_menu_item waypoint_create = {
		"Create",
		[&]()
		{
			add_waypoint();
		},
		[&]()
		{
			return true;
		},
		"Ctrl+Shift+W",
		GLFW_KEY_L,
		GLFW_MOD_CONTROL | GLFW_MOD_SHIFT,
	};
	shortcut_menu_item waypoint_duplicate = {
		"Duplicate",
		[&]()
		{
			duplicate_selected_waypoint();
		},
		[&]()
		{
			return get_selected_waypoint();
		},
		"Ctrl+D",
		GLFW_KEY_D,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item waypoint_rename = {
		"Rename",
		[&]()
		{
			assert(m_sg_window);
			assert(!m_sg_window->get_renaming_waypoint());
			m_sg_window->set_renaming_waypoint(get_selected_waypoint());
		},
		[&]()
		{
			return get_selected_waypoint();
		},
		"Ctrl+R",
		GLFW_KEY_R,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item waypoint_destroy = {
		"Destroy",
		[&]()
		{
			waypoint* const l = get_selected_waypoint();
			destroy_waypoint(l);
			set_selected_waypoint(nullptr);
		},
		[&]()
		{
			return get_selected_waypoint();
		},
		"Delete",
		GLFW_KEY_DELETE,
		0,
	};

	shortcut_menu waypoint_menu;
	waypoint_menu.name = "Waypoints";
	waypoint_menu.groups.push_back({ waypoint_create });
	waypoint_menu.groups.push_back({ waypoint_duplicate });
	waypoint_menu.groups.push_back({ waypoint_rename, waypoint_destroy });
	shortcut_menus.push_back(waypoint_menu);
}
