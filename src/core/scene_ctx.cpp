#include "pch.h"
#include "scene_ctx.h"
#include "sgnode.h"
#include "scene_material.h"
#include "geom/carve.h"
#include "geom/generated_mesh.h"
#include "smnode.h"
#include "ui/app_ctx.h"

scene_ctx::scene_ctx() :
	m_light_buffer(sizeof(mgl::light) * s_num_lights)
{
	clear(false);
}
scene_ctx::~scene_ctx()
{
	destroy();
}



carve::csg::CSG& scene_ctx::get_csg()
{
	return m_csg;
}
carve_vert_attrs& scene_ctx::get_vert_attrs()
{
	return m_vert_attrs;
}
attr_material_t& scene_ctx::get_mtl_id_attr()
{
	return m_mtl_id_attr;
}
const carve_vert_attrs& scene_ctx::get_vert_attrs() const
{
	return m_vert_attrs;
}
const attr_material_t& scene_ctx::get_mtl_id_attr() const
{
	return m_mtl_id_attr;
}
void scene_ctx::draw(const mgl::context& glctx, const scene_ctx_uniforms& mats)
{
	m_skybox->draw(mats.view, mats.proj);
	m_draw_vaos(glctx, mats, m_sg_ros_for_mtl);
	for (const auto& pair : m_sm_ros)
	{
		if (pair.first->is_visible())
			m_draw_vaos(glctx, mats, pair.second, pair.first->get_mat(), pair.first->get_uv_offset());
	}
}
void scene_ctx::update(app_ctx* const app)
{
	if (m_sg_root->is_dirty())
	{
		m_sg_root->recompute(this);
		m_build_sg_vaos();
	}

	m_build_sm_vaos();

	assert(m_lights.size() >= 1);
	light* const cam_light = m_lights[0];
	if (cam_light->is_visible())
	{
		const tmat<space::CAMERA, space::WORLD>& cam_mat = app->preview_cam.get_view().invert_copy();
		cam_light->set_mat(cam_mat.cast_copy<space::OBJECT, space::WORLD>());
		m_build_light_buffer();
	}
}
void scene_ctx::clear(bool ready_for_default_material)
{
	xportable::reset_next_id();
	xportable::reset_num_tags_created();
	s_next_mtl_id = 1;

	m_csg = carve::csg::CSG();
	m_vert_attrs = carve_vert_attrs();
	m_mtl_id_attr = attr_material_t();

	m_vert_attrs.install_hooks(m_csg);
	m_mtl_id_attr.installHooks(m_csg);
	m_sg_root = new sgnode(nullptr, carve::csg::CSG::OP::UNION);

	if (g::texlib)
	{
		g::texlib->set_autotexture_cleanup_enabled(false);
	}
	for (const auto& pair : m_mtls)
		delete pair.second;
	m_mtls.clear();
	if (g::texlib)
	{
		g::texlib->set_autotexture_cleanup_enabled(true);
	}

	for (smnode* const sm : m_static_meshes)
		delete sm;
	m_static_meshes.clear();
	m_sm_ros.clear();

	for (light* const l : m_lights)
		delete l;
	m_lights.clear();
	// camera light
	add_light(new light({}, "Camera Light"));
	const auto& light_params = m_lights[0]->get_params();
	for (const auto& param : light_params)
	{
		if (param.first == "Max Distance")
		{
			*static_cast<float*>(param.second.value) = 100000.0f;
		}
		if (param.first == "Ambient Color")
		{
			static_cast<float*>(param.second.value)[3] = 0.3f;
		}
	}

	for (waypoint* const w : m_waypoints)
		delete w;
	m_waypoints.clear();

	// needs to happen after mtls are deleted (they unload textures from texlib)
	// and before new mtl is created (needs new shaders)
	g::clear();

	if (ready_for_default_material)
	{
		scene_material* const null_mtl = create_default_material();
		null_mtl->set_name(g::null_mtl_name);
		m_mtls.insert(std::make_pair(0, null_mtl));

		load_skybox("", "");
	}
}
void scene_ctx::destroy()
{
	delete m_sg_root;
	m_sg_root = nullptr;

	if (g::texlib)
	{
		g::texlib->set_autotexture_cleanup_enabled(false);
	}
	for (const auto& pair : m_mtls)
		delete pair.second;
	m_mtls.clear();
	if (g::texlib)
	{
		g::texlib->set_autotexture_cleanup_enabled(true);
	}

	delete m_skybox;
	m_skybox = nullptr;
}
void scene_ctx::save(std::ofstream& out, const std::string& out_fp)
{
	nlohmann::json obj;

	obj["rid"] = m_sg_root->get_id();
	if (m_skybox_folder.empty())
	{
		obj["sb"] = "";
	}
	else
	{
		obj["sb"] = u::absolute_to_relative(m_skybox_folder, out_fp);
	}

	obj["mid"] = s_next_mtl_id;
	obj["nm"] = m_mtls.size();
	std::vector<nlohmann::json::array_t> mtls;
	for (const auto& pair : m_mtls)
	{
		mtls.push_back({ pair.first, pair.second->save(out, out_fp) });
	}
	obj["m"] = mtls;

	obj["ns"] = m_static_meshes.size();
	std::vector<nlohmann::json> sms;
	for (const smnode* const sm : m_static_meshes)
	{
		sms.push_back(sm->save(this));
	}
	obj["s"] = sms;

	obj["nl"] = m_lights.size();
	std::vector<nlohmann::json> ls;
	for (const light* const l : m_lights)
	{
		ls.push_back(l->save());
	}
	obj["l"] = ls;

	obj["nw"] = m_waypoints.size();
	std::vector<nlohmann::json> ws;
	for (const waypoint* const w : m_waypoints)
	{
		ws.push_back(w->save());
	}
	obj["w"] = ws;

	out << obj << "\n";
}
const std::string scene_ctx::load(std::ifstream& in, const std::string& in_fp)
{
	const nlohmann::json& obj = u::next_line_json(in);

	s_next_mtl_id = obj["mid"];
	m_mtls.reserve(obj["nm"]);
	for (const nlohmann::json::array_t& mtl : obj["m"])
	{
		const u32 id = mtl[0];
		scene_material* const sm = new scene_material(in_fp, mtl[1], g::opaque_shaders, g::alpha_shaders);
		m_mtls.insert({ id, sm });
	}

	m_static_meshes.reserve(obj["ns"]);
	for (const nlohmann::json& sm : obj["s"])
	{
		smnode* const node = new smnode(sm, this);
		m_static_meshes.push_back(node);
		m_sm_ros.insert({ node, {} });
	}
	m_build_sm_vaos();

	// need to erase the default camera light before adding the one for this scene
	for (light* const l : m_lights)
		delete l;
	m_lights.clear();

	m_lights.reserve(obj["nl"]);
	for (const nlohmann::json& l : obj["l"])
	{
		m_lights.emplace_back(new light(l));
	}
	m_build_light_buffer();


	m_waypoints.reserve(obj["nw"]);
	for (const nlohmann::json& wp : obj["w"])
	{
		m_waypoints.emplace_back(new waypoint(wp));
	}

	load_skybox(obj["sb"], in_fp);

	return obj["rid"];
}
void scene_ctx::xport_sgnode(sgnode* const cur, std::vector<std::unordered_map<u32, mgl::static_retained_render_object>*>* const phorm_ros)
{
	std::unordered_map<u32, std::vector<mesh_vertex>> verts_for_mtl;
	std::unordered_map<u32, std::vector<u32>> indices_for_mtl;
	for (auto it = m_mtls.begin(); it != m_mtls.end(); ++it)
	{
		verts_for_mtl.insert(std::make_pair(it->first, std::vector<mesh_vertex>()));
		indices_for_mtl.insert(std::make_pair(it->first, std::vector<u32>()));
	}
	const generated_mesh* const gen = cur->compute_xport(this);
	m_tesselate(gen->mesh, verts_for_mtl, indices_for_mtl, true);

	const tmat<space::PARENT, space::WORLD>& mat = cur->accumulate_parent_mats();
	phorm_ros->emplace_back(new std::unordered_map<u32, mgl::static_retained_render_object>());
	for (auto it = m_mtls.begin(); it != m_mtls.end(); ++it)
	{
		auto& verts = verts_for_mtl.at(it->first);
		if (verts.empty())
			continue;
		for (mesh_vertex& vert : verts)
		{
			// vert.transform(mat);
		}
		const auto& indices = indices_for_mtl.at(it->first);
		mgl::static_retained_render_object ro((f32*)verts.data(), (u32)verts.size(), get_vert_layout(), (u32*)indices.data(), (u32)indices.size());
		phorm_ros->back()->emplace(it->first, std::move(ro));
	}
	delete gen;
}
void scene_ctx::save_xport(haul::output_file* const out)
{
	// texturez
	std::unordered_map<std::string, u64> texname2idx;
	out->put64(g::texlib->get_all().size() + 1);
	printf("t: %zu\n", g::texlib->get_all().size() + 1);
	g::texlib->get_deftex()->save(out);
	texname2idx.insert({ g::null_tex_fp, 0 });
	u64 textures_xported = 1; // index 0 represents <NULL> texture
	for (const auto& pair : g::texlib->get_all())
	{
		pair.second->save(out);
		texname2idx.insert({ pair.first, textures_xported });
		textures_xported++;
	}

	// skybox
	m_skybox->get_texture().save(out);

	// materialz
	out->put64(m_mtls.size());
	printf("m: %zu\n", m_mtls.size());
	for (const auto& pair : m_mtls)
	{
		out->put32(pair.first);
		pair.second->xport(out, texname2idx);
	}

	// phormz
	std::vector<std::unordered_map<u32, mgl::static_retained_render_object>*> phorm_ros;
	std::vector<sgnode*> phorms;
	std::vector<sgnode*> stack;
	stack.push_back(m_sg_root);
	while (!stack.empty())
	{
		sgnode* const cur = stack.back();
		stack.pop_back();

		if (cur->is_separate_xport())
		{
			xport_sgnode(cur, &phorm_ros);
			phorms.push_back(cur);
		}
		else
		{
			const auto& children = cur->get_children();
			stack.insert(stack.end(), children.begin(), children.end());
		}
	}
	xport_sgnode(m_sg_root, &phorm_ros);
	phorms.push_back(m_sg_root);
	assert(phorm_ros.size() == phorms.size());
	out->put64(phorm_ros.size());
	printf("sg: %zu\n", phorm_ros.size());
	for (u64 i = 0; i < phorm_ros.size(); i++)
	{
		phorms[i]->xport(out);
		const auto& phorm = phorm_ros[i];
		out->put64(phorm->size());
		for (const auto& pair : *phorm)
		{
			out->put32(pair.first);
			pair.second.save(out);
		}
	}

	// smnodes
	out->put64(m_sm_ros.size());
	printf("sm: %zu\n", m_sm_ros.size());
	for (const auto& pair : m_sm_ros)
	{
		pair.first->xport(out);
		const auto& materials = m_build_sm_vaos_world(pair.first);
		out->put64(materials.size());
		for (const auto& material : materials)
		{
			out->put32(material.first);
			material.second.save(out);
		}
	}

	// lightz (skip camera light)
	out->put64(m_lights.size() - 1);
	printf("l: %zu\n", m_lights.size() - 1);
	for (u64 i = 1; i < m_lights.size(); i++)
	{
		m_lights[i]->xport(out);
	}

	// waypointz
	out->put64(m_waypoints.size());
	printf("w: %zu\n", m_waypoints.size());
	for (const waypoint* const w : m_waypoints)
	{
		w->xport(out);
	}
}
void scene_ctx::load_skybox(const std::string& rel_folder, const std::string& phorm_base)
{
	delete m_skybox;
	if (rel_folder.empty())
	{
		m_skybox_folder = "";
		m_skybox = u::load_retained_skybox_rgb_u8("src/glsl/sky.vert", "src/glsl/sky.frag",
			{
				"res/skybox/px.png",
				"res/skybox/nx.png",
				"res/skybox/py.png",
				"res/skybox/ny.png",
				"res/skybox/pz.png",
				"res/skybox/nz.png",
			});
	}
	else
	{
		std::filesystem::path folder(phorm_base.empty() ? rel_folder : u::relative_to_absolute(rel_folder, phorm_base));
		m_skybox_folder = folder.string();
		m_skybox = u::load_retained_skybox_rgb_u8("src/glsl/sky.vert", "src/glsl/sky.frag",
			{
				(folder / "px.png").string(),
				(folder / "nx.png").string(),
				(folder / "py.png").string(),
				(folder / "ny.png").string(),
				(folder / "pz.png").string(),
				(folder / "nz.png").string(),
			});
	}
}



sgnode* scene_ctx::get_sg_root()
{
	return m_sg_root;
}
const sgnode* scene_ctx::get_sg_root() const
{
	return m_sg_root;
}
void scene_ctx::set_sg_root(sgnode* const new_root)
{
	delete m_sg_root;
	m_sg_root = new_root;
	// m_selected_node = new_root;
}



const std::unordered_map<u32, scene_material*>& scene_ctx::get_materials()
{
	return m_mtls;
}
scene_material* scene_ctx::create_default_material()
{
	scene_material* mtl = new scene_material;
	mtl->opaque_shaders = g::opaque_shaders;
	mtl->alpha_shaders = g::alpha_shaders;
	mtl->set_texture("u_tex0", g::null_tex_fp);
	mtl->set_texture("u_tex1", g::null_tex_fp);
	mtl->set_texture("u_tex2", g::null_tex_fp);
	mtl->set_texture("u_tex3", g::null_tex_fp);
	return mtl;
}
u32 scene_ctx::add_material(scene_material* const mtl)
{
	const u32 id = s_next_mtl_id;
	m_mtls.insert(std::make_pair(id, mtl));
	++s_next_mtl_id;
	return id;
}
void scene_ctx::erase_material(const u32 id)
{
	delete m_mtls[id];
	m_mtls.erase(id);
}
u32 scene_ctx::get_id_for_material(scene_material* const mat)
{
	for (const auto& pair : m_mtls)
	{
		if (pair.second == mat)
		{
			return pair.first;
		}
	}
	assert(false);
	return 0;
}
scene_material* scene_ctx::get_material(const GLuint id)
{
	return m_mtls[id];
}



std::vector<light*>& scene_ctx::get_lights()
{
	return m_lights;
}
light* scene_ctx::add_light()
{
	return add_light(new light());
}
light* scene_ctx::add_light(light* const l)
{
	if (m_lights.size() < s_num_lights)
	{
		m_lights.push_back(l);
		/*const light* const l = m_lights.back();
		m_light_buffer.update((f32*)&l->mgl_light, sizeof(mgl::light) / sizeof(f32), sizeof(mgl::light) * u32(m_lights.size() - 1));*/
		m_build_light_buffer();
		return m_lights.back();
	}
	return nullptr;
}
void scene_ctx::destroy_light(light* const l)
{
	const auto& it = std::find(m_lights.begin(), m_lights.end(), l);
	assert(it != m_lights.end());
	m_lights.erase(it);
	delete l;
	// I'm lazy
	m_build_light_buffer();
}
void scene_ctx::update_light(const light* const l)
{
	const auto& it = std::find(m_lights.begin(), m_lights.end(), l);
	assert(it != m_lights.end());
	/*const u32 index = u32(it - m_lights.begin());
	m_light_buffer.update((f32*)&l->mgl_light, sizeof(mgl::light) / sizeof(f32), index * sizeof(mgl::light));*/
	m_build_light_buffer();
}



std::vector<waypoint*>& scene_ctx::get_waypoints()
{
	return m_waypoints;
}
waypoint* scene_ctx::add_waypoint()
{
	return add_waypoint(new waypoint());
}
waypoint* scene_ctx::add_waypoint(waypoint* const w)
{
	m_waypoints.push_back(w);
	return w;
}
void scene_ctx::destroy_waypoint(waypoint* const w)
{
	const auto& it = std::find(m_waypoints.begin(), m_waypoints.end(), w);
	assert(it != m_waypoints.end());
	m_waypoints.erase(it);
	delete w;
}



const std::vector<smnode*>& scene_ctx::get_static_meshes()
{
	return m_static_meshes;
}
smnode* const scene_ctx::add_static_mesh()
{
	smnode* const result = new smnode(generated_textured_heightmap(0));
	m_static_meshes.push_back(result);
	m_sm_ros.insert({ result, {} });
	m_build_sm_vaos();
	return result;
}
void scene_ctx::add_static_mesh(smnode* const node)
{
	m_static_meshes.push_back(node);
	m_sm_ros.insert({ node, {} });
	m_build_sm_vaos();
}
void scene_ctx::destroy_static_mesh(smnode* const n)
{
	const auto& it = std::find(m_static_meshes.begin(), m_static_meshes.end(), n);
	assert(it != m_static_meshes.end());
	m_static_meshes.erase(it);
	m_sm_ros.erase(n);
	delete n;
	m_build_sm_vaos();
}



mesh_t* scene_ctx::create_textured_cuboid(const GLuint mtl_id, const cuboid_options& options)
{
	return textured_cuboid(m_vert_attrs, m_mtl_id_attr, mtl_id, options);
}
mesh_t* scene_ctx::create_textured_ellipsoid(const GLuint mtl_id, const ellipsoid_options& options)
{
	return textured_ellipsoid(m_vert_attrs, m_mtl_id_attr, mtl_id, options);
}
mesh_t* scene_ctx::create_textured_cylinder(const GLuint mtl_id, const cylinder_options& options)
{
	return textured_cylinder(m_vert_attrs, m_mtl_id_attr, mtl_id, options);
}
mesh_t* scene_ctx::create_textured_cone(const GLuint mtl_id, const cone_options& options)
{
	return textured_cone(m_vert_attrs, m_mtl_id_attr, mtl_id, options);
}
mesh_t* scene_ctx::create_textured_torus(const GLuint mtl_id, const torus_options& options)
{
	return textured_torus(m_vert_attrs, m_mtl_id_attr, mtl_id, options);
}
mesh_t* scene_ctx::create_textured_heightmap(const GLuint mtl_id, const heightmap_options& options)
{
	return textured_heightmap(m_vert_attrs, m_mtl_id_attr, mtl_id, options);
}
generated_mesh* scene_ctx::generated_textured_cuboid(const GLuint mtl_id, const cuboid_options& options)
{
	return new generated_cuboid(this, mtl_id, options);
}
generated_mesh* scene_ctx::generated_textured_ellipsoid(const GLuint mtl_id, const ellipsoid_options& options)
{
	return new generated_ellipsoid(this, mtl_id, options);
}
generated_mesh* scene_ctx::generated_textured_cylinder(const GLuint mtl_id, const cylinder_options& options)
{
	return new generated_cylinder(this, mtl_id, options);
}
generated_mesh* scene_ctx::generated_textured_cone(const GLuint mtl_id, const cone_options& options)
{
	return new generated_cone(this, mtl_id, options);
}
generated_mesh* scene_ctx::generated_textured_torus(const GLuint mtl_id, const torus_options& options)
{
	return new generated_torus(this, mtl_id, options);
}
generated_mesh* scene_ctx::generated_textured_heightmap(const GLuint mtl_id, const heightmap_options& options)
{
	return new generated_heightmap(this, mtl_id, options);
}
generated_mesh* scene_ctx::generated_textured_heightmap_static(const GLuint mtl_id, const heightmap_options& options)
{
	mesh_t* const hm = textured_heightmap(m_vert_attrs, m_mtl_id_attr, mtl_id, options);
	return new generated_static_mesh(hm, this);
}
void scene_ctx::tesselate_external(const mesh_t* mesh, std::unordered_map<u32, std::vector<mesh_vertex>>& out_verts_for_mtl)
{
	GLUtesselator* tess = gluNewTess();
	gluTessCallback(tess, GLU_TESS_BEGIN, (GLUTessCallback)tess_callback_begin);
	gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (GLUTessCallback)(tess_callback_vertex_data));
	gluTessCallback(tess, GLU_TESS_EDGE_FLAG, (GLUTessCallback)tess_callback_edge_flag); // Edge flag forces only triangles
	gluTessCallback(tess, GLU_TESS_END, (GLUTessCallback)tess_callback_end);
	gluTessCallback(tess, GLU_TESS_ERROR, (GLUTessCallback)tess_callback_error);
	if (mesh)
	{
		for (mesh_t::const_face_iter i = mesh->faceBegin(); i != mesh->faceEnd(); ++i)
		{
			const mesh_t::face_t* f = *i;
			u32 mtl_id = m_mtl_id_attr.getAttribute(f, 0);

			std::vector<tess_vtx> verts;
			for (mesh_t::face_t::const_edge_iter_t e = f->begin(); e != f->end(); ++e)
			{
				const tex_coord_t& t0 = m_vert_attrs.uv0.getAttribute(f, e.idx());
				const tex_coord_t& t1 = m_vert_attrs.uv1.getAttribute(f, e.idx());
				const tex_coord_t& t2 = m_vert_attrs.uv2.getAttribute(f, e.idx());
				const tex_coord_t& t3 = m_vert_attrs.uv3.getAttribute(f, e.idx());
				const f64 w0 = m_vert_attrs.w0.getAttribute(f, e.idx());
				const f64 w1 = m_vert_attrs.w1.getAttribute(f, e.idx());
				const f64 w2 = m_vert_attrs.w2.getAttribute(f, e.idx());
				const f64 w3 = m_vert_attrs.w3.getAttribute(f, e.idx());
				const color_t& color = m_vert_attrs.color.getAttribute(f, e.idx());

				tess_vtx v(e->vert->v.x, e->vert->v.y, e->vert->v.z, t0, t1, t2, t3, w0, w1, w2, w3, color);
				v.target = &out_verts_for_mtl.at(mtl_id);
				verts.emplace_back(v);
			}

			gluTessBeginPolygon(tess, nullptr);
			gluTessBeginContour(tess);
			for (const tess_vtx& v : verts)
				gluTessVertex(tess, (GLdouble*)&v, (GLvoid*)&v);
			gluTessEndContour(tess);
			gluTessEndPolygon(tess);
		}
	}
	gluDeleteTess(tess);
}


void scene_ctx::m_build_light_buffer()
{
	m_num_visible_lights = 0;
	std::vector<mgl::light> mgl_lights;
	mgl_lights.reserve(m_lights.size());

	for (const light* const l : m_lights)
	{
		if (l->is_visible())
		{
			mgl_lights.push_back(l->mgl_light);
			m_num_visible_lights++;
		}
	}
	m_light_buffer.update((f32*)mgl_lights.data(), sizeof(mgl::light) / sizeof(f32) * (u32)mgl_lights.size(), 0);
}
void scene_ctx::m_build_sg_vaos()
{
	std::unordered_map<u32, std::vector<mesh_vertex>> verts_for_mtl;
	std::unordered_map<u32, std::vector<u32>> indices_for_mtl;
	for (auto it = m_mtls.begin(); it != m_mtls.end(); ++it)
	{
		verts_for_mtl.insert(std::make_pair(it->first, std::vector<mesh_vertex>()));
		indices_for_mtl.insert(std::make_pair(it->first, std::vector<u32>()));
	}

	const generated_mesh* gen = m_sg_root->get_gen();
	if (gen)
	{
		m_tesselate(gen->mesh, verts_for_mtl, indices_for_mtl, true);
	}

	m_sg_ros_for_mtl.clear();
	for (auto it = m_mtls.begin(); it != m_mtls.end(); ++it)
	{
		const auto& verts = verts_for_mtl.at(it->first);
		if (verts.empty())
			continue;
		const auto& indices = indices_for_mtl.at(it->first);
		mgl::static_retained_render_object ro((f32*)verts.data(), (u32)verts.size(), get_vert_layout(), (u32*)indices.data(), (u32)indices.size());
		m_sg_ros_for_mtl.emplace(it->first, std::move(ro));
	}
}
void scene_ctx::m_build_sm_vaos()
{
	for (auto& sm2ro : m_sm_ros)
	{
		smnode* const node = sm2ro.first;
		if (!node->is_gen_dirty())
			continue;
		sm2ro.second.clear();
		node->recompute(this);

		std::unordered_map<u32, std::vector<mesh_vertex>> verts_for_mtl;
		std::unordered_map<u32, std::vector<u32>> indices_for_mtl;
		const mesh_t* const mesh = node->get_mesh();
		for (mesh_t::const_face_iter i = mesh->faceBegin(); i != mesh->faceEnd(); ++i)
		{
			const mesh_t::face_t* f = *i;
			u32 mtl_id = m_mtl_id_attr.getAttribute(f, 0);
			if (!indices_for_mtl.contains(mtl_id))
				indices_for_mtl.insert(std::make_pair(mtl_id, std::vector<u32>()));

			std::vector<tess_vtx> verts;
			for (mesh_t::face_t::const_edge_iter_t e = f->begin(); e != f->end(); ++e)
			{
				const tex_coord_t& t0 = m_vert_attrs.uv0.getAttribute(f, e.idx());
				const tex_coord_t& t1 = m_vert_attrs.uv1.getAttribute(f, e.idx());
				const tex_coord_t& t2 = m_vert_attrs.uv2.getAttribute(f, e.idx());
				const tex_coord_t& t3 = m_vert_attrs.uv3.getAttribute(f, e.idx());
				const f64 w0 = m_vert_attrs.w0.getAttribute(f, e.idx());
				const f64 w1 = m_vert_attrs.w1.getAttribute(f, e.idx());
				const f64 w2 = m_vert_attrs.w2.getAttribute(f, e.idx());
				const f64 w3 = m_vert_attrs.w3.getAttribute(f, e.idx());
				const color_t& color = m_vert_attrs.color.getAttribute(f, e.idx());

				verts_for_mtl[mtl_id].emplace_back((f32)e->vert->v.x, (f32)e->vert->v.y, (f32)e->vert->v.z, t0, t1, t2, t3, (f32)w0, (f32)w1, (f32)w2, (f32)w3, color);
			}
		}

		const bool snap_norms = true;
		for (auto& pair : verts_for_mtl)
		{
			if (node->should_snap())
				m_compute_norms_snap(pair.second, indices_for_mtl.at(pair.first), node->should_snap_all(), node->get_snap_angle());
			else
				m_compute_norms(pair.second, indices_for_mtl.at(pair.first));
		}

		for (auto it = verts_for_mtl.begin(); it != verts_for_mtl.end(); ++it)
		{
			const auto& verts = it->second;
			if (verts.empty())
				continue;
			const auto& indices = indices_for_mtl.at(it->first);
			mgl::static_retained_render_object ro((f32*)verts.data(), (u32)verts.size(), get_vert_layout(), (u32*)indices.data(), (u32)indices.size());
			m_sm_ros[sm2ro.first].emplace(it->first, std::move(ro));
		}
	}
}
std::unordered_map<u32, mgl::static_retained_render_object> scene_ctx::m_build_sm_vaos_world(smnode* const node)
{
	node->recompute(this);

	std::unordered_map<u32, std::vector<mesh_vertex>> verts_for_mtl;
	std::unordered_map<u32, std::vector<u32>> indices_for_mtl;
	const mesh_t* const mesh = node->get_mesh();
	for (mesh_t::const_face_iter i = mesh->faceBegin(); i != mesh->faceEnd(); ++i)
	{
		const mesh_t::face_t* f = *i;
		u32 mtl_id = m_mtl_id_attr.getAttribute(f, 0);
		if (!indices_for_mtl.contains(mtl_id))
			indices_for_mtl.insert(std::make_pair(mtl_id, std::vector<u32>()));

		std::vector<tess_vtx> verts;
		for (mesh_t::face_t::const_edge_iter_t e = f->begin(); e != f->end(); ++e)
		{
			const tex_coord_t& t0 = m_vert_attrs.uv0.getAttribute(f, e.idx());
			const tex_coord_t& t1 = m_vert_attrs.uv1.getAttribute(f, e.idx());
			const tex_coord_t& t2 = m_vert_attrs.uv2.getAttribute(f, e.idx());
			const tex_coord_t& t3 = m_vert_attrs.uv3.getAttribute(f, e.idx());
			const f64 w0 = m_vert_attrs.w0.getAttribute(f, e.idx());
			const f64 w1 = m_vert_attrs.w1.getAttribute(f, e.idx());
			const f64 w2 = m_vert_attrs.w2.getAttribute(f, e.idx());
			const f64 w3 = m_vert_attrs.w3.getAttribute(f, e.idx());
			const color_t& color = m_vert_attrs.color.getAttribute(f, e.idx());

			verts_for_mtl[mtl_id].emplace_back((f32)e->vert->v.x, (f32)e->vert->v.y, (f32)e->vert->v.z, t0, t1, t2, t3, (f32)w0, (f32)w1, (f32)w2, (f32)w3, color);
		}
	}

	const bool snap_norms = true;
	for (auto& pair : verts_for_mtl)
	{
		if (node->should_snap())
			m_compute_norms_snap(pair.second, indices_for_mtl.at(pair.first), node->should_snap_all(), node->get_snap_angle());
		else
			m_compute_norms(pair.second, indices_for_mtl.at(pair.first));
	}

	const tmat<space::OBJECT, space::WORLD>& mat = node->get_mat();
	const tex_coord_t* const offset = node->get_uv_offset();

	std::unordered_map<u32, mgl::static_retained_render_object> ret;
	for (auto it = verts_for_mtl.begin(); it != verts_for_mtl.end(); ++it)
	{
		auto& verts = it->second;
		if (verts.empty())
			continue;
		for (mesh_vertex& vert : verts)
		{
			vert.transform(mat);
			if (offset)
				vert.apply_uv_offset(offset);
		}
		const auto& indices = indices_for_mtl.at(it->first);
		mgl::static_retained_render_object ro((f32*)verts.data(), (u32)verts.size(), get_vert_layout(), (u32*)indices.data(), (u32)indices.size());
		ret.emplace(it->first, std::move(ro));
	}
	return ret;
}
void scene_ctx::m_tesselate(const mesh_t* mesh, std::unordered_map<u32, std::vector<mesh_vertex>>& out_verts_for_mtl, std::unordered_map<u32, std::vector<u32>>& out_indices_for_mtl, const bool snap_norms)
{
	tesselate_external(mesh, out_verts_for_mtl);
	for (auto& pair : out_verts_for_mtl)
	{
		if (snap_norms)
			m_compute_norms_snap(pair.second, out_indices_for_mtl.at(pair.first), s_snap_all, s_snap_angle);
		else
			m_compute_norms(pair.second, out_indices_for_mtl.at(pair.first));
	}
}
void scene_ctx::m_draw_vaos(const mgl::context& glctx, const scene_ctx_uniforms& mats, const std::unordered_map<u32, mgl::static_retained_render_object>& ros, const tmat<space::OBJECT, space::WORLD>& model, const tex_coord_t* offset)
{
	const tmat<space::OBJECT, space::CAMERA>& mv = mats.view * model;
	const mat<space::OBJECT, space::CLIP> mvp = mats.vp * model;
	const tmat<space::OBJECT, space::WORLD>& normal = model.invert_copy().transpose_copy();

	m_light_buffer.bind(0);
	for (auto it = ros.begin(); it != ros.end(); ++it)
	{
		const scene_material* mat = m_mtls[it->first];
		if (mat->get_should_cull())
		{
			glEnable(GL_CULL_FACE);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}
		mgl::shaders* s = mat->get_use_alpha() ? mat->alpha_shaders : mat->opaque_shaders;
		s->bind();
		if (s->has_uniform("u_mvp"))
			s->uniform_mat4("u_mvp", mvp.e);
		if (s->has_uniform("u_mv"))
			s->uniform_mat4("u_mv", mv.e);
		if (s->has_uniform("u_m"))
			s->uniform_mat4("u_m", model.e);
		if (s->has_uniform("u_normal"))
			s->uniform_mat4("u_normal", normal.e);
		if (s->has_uniform("u_cam_pos"))
			s->uniform_3fv("u_cam_pos", mats.cam_pos.e);
		if (s->has_uniform("u_time"))
			s->uniform_1f("u_time", glctx.time.now);
		if (s->has_uniform("u_num_lights"))
			s->uniform_1ui("u_num_lights", m_num_visible_lights);
		if (s->has_uniform("u_enable_lighting"))
			s->uniform_1i("u_enable_lighting", mat->get_use_lighting());
		if (offset)
		{
			if (s->has_uniform("u_uv0_offset"))
				s->uniform_4f("u_uv0_offset", offset[0].u, offset[0].v, offset[0].uo, offset[0].vo);
			if (s->has_uniform("u_uv1_offset"))
				s->uniform_4f("u_uv1_offset", offset[1].u, offset[1].v, offset[1].uo, offset[1].vo);
			if (s->has_uniform("u_uv2_offset"))
				s->uniform_4f("u_uv2_offset", offset[2].u, offset[2].v, offset[2].uo, offset[2].vo);
			if (s->has_uniform("u_uv3_offset"))
				s->uniform_4f("u_uv3_offset", offset[3].u, offset[3].v, offset[3].uo, offset[3].vo);
		}
		else
		{
			if (s->has_uniform("u_uv0_offset"))
				s->uniform_4f("u_uv0_offset", 1, 1, 0, 0);
			if (s->has_uniform("u_uv1_offset"))
				s->uniform_4f("u_uv1_offset", 1, 1, 0, 0);
			if (s->has_uniform("u_uv2_offset"))
				s->uniform_4f("u_uv2_offset", 1, 1, 0, 0);
			if (s->has_uniform("u_uv3_offset"))
				s->uniform_4f("u_uv3_offset", 1, 1, 0, 0);
		}

		u32 slot = 0;
		mat->for_each_texture([&](const std::string& name, const texture* tex)
			{
				tex->bind(slot);
				s->uniform_1i(name.c_str(), slot);
				++slot;
			});

		glctx.draw(it->second, *s);
	}

	glEnable(GL_CULL_FACE);
}
void scene_ctx::m_compute_norms_snap(std::vector<mesh_vertex>& input_verts, std::vector<u32>& indices, const bool snap_all, const f32 snap_angle)
{
	std::vector<u32> output_vert2index;
	std::unordered_map<std::string, std::unordered_map<u32, std::vector<direction<space::OBJECT>>>> vert2index;
	std::vector<mesh_vertex> unique_verts;
	u32 index_count = 0;
	assert(input_verts.size() % 3 == 0);
	for (u32 i = 0; i < input_verts.size(); i += 3)
	{
		const mesh_vertex& va = input_verts[i + 0];
		const mesh_vertex& vb = input_verts[i + 1];
		const mesh_vertex& vc = input_verts[i + 2];
		// vertex positions of current triangle
		const vec<space::OBJECT> pa(va.x, va.y, va.z);
		const vec<space::OBJECT> pb(vb.x, vb.y, vb.z);
		const vec<space::OBJECT> pc(vc.x, vc.y, vc.z);
		// sides of current triangle
		const vec<space::OBJECT>& ab = pa - pb;
		const vec<space::OBJECT>& ac = pa - pc;
		// face normal of current triangle
		const direction<space::OBJECT> norm(ab.cross_copy(ac));
		// go through the verts in current face
		for (u32 j = 0; j < 3; j++)
		{
			const mesh_vertex& mv = input_verts[i + j];
			const std::string& key = mv.hash_all();
			const auto& it = vert2index.find(key);
			// this vertex has been seen before, try to match it to existing instance
			bool found = false;
			if (it != vert2index.end())
			{
				// check existing instances of this vertex
				for (auto& instance : it->second)
				{
					// check all faces that each instance is part of
					for (u32 k = 0; k < instance.second.size(); k++)
					{
						const auto& face_norm = instance.second.at(k);
						const f32 angle = norm.angle_to(face_norm);
						// angle between current face and existing instance face is sufficiently small to consider them the same face
						// (this is required because all faces are triangles, so some triangles making up a single planar face could have their normals counted twice)
						if (abs(angle) < c::EPSILON)
						{
							indices.push_back(instance.first);
							goto next_vertex;
						}
						if (snap_all)
						{
							// current vertex cannot be added to this instance (ALL)
							if (fabs(angle) >= snap_angle)
							{
								break;
							}
							// made it to the end of the list, current vertex is part of this instance
							if (k == instance.second.size() - 1)
							{
								// vert2index.at(key).at(instance.first).push_back(norm);
								instance.second.push_back(norm);
								indices.push_back(instance.first);
								found = true;
								// need to break here because we're adding to instance.second, so this loop will go forever
								break;
							}
						}
						else
						{
							// current vertex can be added to this instance (ANY)
							if (fabs(angle) < snap_angle)
							{
								// vert2index.at(key).at(instance.first).push_back(norm);
								instance.second.push_back(norm);
								indices.push_back(instance.first);
								found = true;
								// need to break here because we're adding to instance.second, so this loop will go forever
								break;
							}
						}
					}
					// current vertex inserted, stop
					if (found)
						break;
				}
			}
			// either this vertex hasn't been seen before or it doesn't match any existing instances, so make a new one
			if (!found)
			{
				// totally new vertex, need to create map
				if (!vert2index.contains(key))
				{
					vert2index.insert({
						key,
						{ { index_count, { norm } } },
					});
				}
				// at least one instance of this vertex already exists, just add to the map
				else
				{
					vert2index.at(key).insert({ index_count, { norm } });
				}
				indices.push_back(index_count);
				output_vert2index.push_back(index_count);
				index_count++;
				unique_verts.push_back(mv);
			}
next_vertex:
			continue;
		}
	}

	// compute weighted norms
	std::unordered_map<u32, vec<space::OBJECT>> norms;
	for (u32 i = 0; i < indices.size(); i += 3)
	{
		// indices of unique vertices of current triangle
		const u32 ia = indices[i + 0];
		const u32 ib = indices[i + 1];
		const u32 ic = indices[i + 2];
		// vertices of current triangle
		const mesh_vertex& va = unique_verts[ia];
		const mesh_vertex& vb = unique_verts[ib];
		const mesh_vertex& vc = unique_verts[ic];
		// vertex positions of current triangle
		const point<space::OBJECT> pa(va.x, va.y, va.z);
		const point<space::OBJECT> pb(vb.x, vb.y, vb.z);
		const point<space::OBJECT> pc(vc.x, vc.y, vc.z);
		// sides of current triangle
		const vec<space::OBJECT>& ab = pa - pb;
		const vec<space::OBJECT>& ac = pa - pc;
		// face normal of current triangle
		const vec<space::OBJECT> norm = ab.cross_copy(ac);
		// add face normal to each vertex. Note that `norm` is not actually normalized, so this inherently weights each normal by the size of the face it is from
		norms[ia] += norm;
		norms[ib] += norm;
		norms[ic] += norm;
	}
	// average weighted norms
	for (auto& pair : norms)
	{
		pair.second.normalize();
	}
	// write norms
	for (u32 i = 0; i < unique_verts.size(); i++)
	{
		mesh_vertex& mv = unique_verts[i];
		const auto& norm = norms[output_vert2index[i]];
		mv.nx = norm.x;
		mv.ny = norm.y;
		mv.nz = norm.z;
	}
	input_verts.clear();
	input_verts = unique_verts;
}
void scene_ctx::m_compute_norms(std::vector<mesh_vertex>& input_verts, std::vector<u32>& indices)
{
	std::unordered_map<std::string, u32> vert2index;
	std::vector<mesh_vertex> unique_verts;
	u32 next_index = 0;
	for (u64 i = 0; i < input_verts.size(); i++)
	{
		const mesh_vertex& mv = input_verts[i];
		const std::string hash = mv.hash_all();
		if (vert2index.contains(hash))
		{
			indices.push_back(vert2index.at(hash));
		}
		else
		{
			unique_verts.push_back(mv);
			indices.push_back(next_index);
			vert2index.insert({ hash, next_index });
			next_index++;
		}
	}
	// compute weighted norms
	std::unordered_map<std::string, vec<space::OBJECT>> norms;
	for (u32 i = 0; i < indices.size(); i += 3)
	{
		// indices of unique vertices of current triangle
		const u32 ia = indices[i + 0];
		const u32 ib = indices[i + 1];
		const u32 ic = indices[i + 2];
		// vertices of current triangle
		const mesh_vertex& va = unique_verts[ia];
		const mesh_vertex& vb = unique_verts[ib];
		const mesh_vertex& vc = unique_verts[ic];
		// vertex positions of current triangle
		const point<space::OBJECT> pa(va.x, va.y, va.z);
		const point<space::OBJECT> pb(vb.x, vb.y, vb.z);
		const point<space::OBJECT> pc(vc.x, vc.y, vc.z);
		// sides of current triangle
		const vec<space::OBJECT>& ab = pa - pb;
		const vec<space::OBJECT>& ac = pa - pc;
		// face normal of current triangle
		const vec<space::OBJECT> norm = ab.cross_copy(ac);
		// add face normal to each vertex. Note that `norm` is not actually normalized, so this inherently weights each normal by the size of the face it is from
		norms[va.hash_pos()] += norm;
		norms[vb.hash_pos()] += norm;
		norms[vc.hash_pos()] += norm;
	}
	// average weighted norms
	for (auto& pair : norms)
	{
		pair.second.normalize();
	}
	// write norms
	for (u32 i = 0; i < unique_verts.size(); i++)
	{
		mesh_vertex& mv = unique_verts[i];
		const auto& norm = norms[mv.hash_pos()];
		mv.nx = norm.x;
		mv.ny = norm.y;
		mv.nz = norm.z;
	}
	input_verts.clear();
	input_verts = unique_verts;
}
