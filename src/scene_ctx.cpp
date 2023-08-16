#include "pch.h"
#include "scene_ctx.h"

scene_ctx::scene_ctx()
{
	m_tex_coord_attr.installHooks(m_csg);
	m_mtl_id_attr.installHooks(m_csg);
}

scene_ctx::~scene_ctx()
{
	delete m_sg_root;
}



u32 scene_ctx::add_heightmap(mesh_t* hm)
{
	m_hms.emplace_back(hm);
	m_hms_dirty = true;
	return (u32)(m_hms.size() - 1);
}

void scene_ctx::remove_heightmap(const u32 id)
{
	m_hms.erase(m_hms.begin() + id);
	m_hms_dirty = true;
}

u32 scene_ctx::add_material(const scene_material& mtl)
{
	const u32 id = s_next_mtl_id;
	m_mtls.insert(std::make_pair(id, mtl));
	++s_next_mtl_id;
	return id;
}

void scene_ctx::remove_material(const u32 id)
{
	m_mtls.erase(id);
}

void scene_ctx::update()
{
	if (m_sg_root->is_dirty())
	{
		m_sg_root->recompute(m_csg);
		m_build_sg_vaos();
	}

	if (m_hms_dirty)
	{
		std::unordered_map<u32, std::vector<mesh_vertex>> verts_for_mtl;
		for (auto it = m_mtls.begin(); it != m_mtls.end(); ++it)
			verts_for_mtl.insert(std::make_pair(it->first, std::vector<mesh_vertex>()));

		for (const auto& hm : m_hms)
			m_tesselate(hm, verts_for_mtl);

		m_hm_vaos_for_mtl.clear();
		for (auto it = verts_for_mtl.begin(); it != verts_for_mtl.end(); ++it)
		{
			// mgl::static_vertex_array vao(it->second.data(), (u32)it->second.size(), { 3, 2 });
			mgl::static_vertex_array vao((f32*)it->second.data(), (u32)it->second.size() * s_vert_size, { 3, 2 });
			// mgl::static_vertex_array vao(it->second.data(), (u32)it->second.size(), { 3, 2, 3 });
			m_hm_vaos_for_mtl.emplace(it->first, std::move(vao));
		}

		m_hms_dirty = false;
	}
}

void scene_ctx::draw(const mgl::context& glctx, const uniform_mats& mats)
{
	m_draw_vaos(glctx, mats, m_hm_vaos_for_mtl);
	m_draw_vaos(glctx, mats, m_sg_vaos_for_mtl);
}



void scene_ctx::m_build_sg_vaos()
{
	std::unordered_map<u32, std::vector<mesh_vertex>> verts_for_mtl;
	for (auto it = m_mtls.begin(); it != m_mtls.end(); ++it)
		verts_for_mtl.insert(std::make_pair(it->first, std::vector<mesh_vertex>()));

	m_tesselate(m_sg_root->mesh, verts_for_mtl);

	m_sg_vaos_for_mtl.clear();
	for (auto it = verts_for_mtl.begin(); it != verts_for_mtl.end(); ++it)
	{
		// mgl::static_vertex_array vao(it->second.data(), (u32)it->second.size(), { 3, 2 });
		mgl::static_vertex_array vao((f32*)it->second.data(), (u32)it->second.size() * s_vert_size, { 3, 2 });
		// mgl::static_vertex_array vao(it->second.data(), (u32)it->second.size(), { 3, 2, 3 });
		m_sg_vaos_for_mtl.emplace(it->first, std::move(vao));
	}
}

void scene_ctx::m_tesselate(const mesh_t* mesh, std::unordered_map<u32, std::vector<mesh_vertex>>& out_verts_for_mtl)
{
	GLUtesselator* tess = gluNewTess();
	gluTessCallback(tess, GLU_TESS_BEGIN, (GLUTessCallback)tess_callback_begin);
	gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (GLUTessCallback)tess_callback_vertex_data);
	gluTessCallback(tess, GLU_TESS_EDGE_FLAG, (GLUTessCallback)tess_callback_edge_flag); // Edge flag forces only triangles
	gluTessCallback(tess, GLU_TESS_END, (GLUTessCallback)tess_callback_end);
	for (mesh_t::const_face_iter i = mesh->faceBegin(); i != mesh->faceEnd(); ++i)
	{
		const mesh_t::face_t* f = *i;
		u32 mtl_id = m_mtl_id_attr.getAttribute(f, 0);

		std::vector<tess_vtx> vtxs;
		for (mesh_t::face_t::const_edge_iter_t e = f->begin(); e != f->end(); ++e)
		{
			auto t = m_tex_coord_attr.getAttribute(f, e.idx());
			tess_vtx v;
			v.x = e->vert->v.x;
			v.y = e->vert->v.y;
			v.z = e->vert->v.z;
			v.u = t.u;
			v.v = t.v;
			v.target = &out_verts_for_mtl[mtl_id];
			vtxs.emplace_back(v);
		}

		gluTessBeginPolygon(tess, nullptr);
		gluTessBeginContour(tess);
		for (const tess_vtx& v : vtxs)
			gluTessVertex(tess, (GLdouble*)&v, (GLvoid*)&v);
		gluTessEndContour(tess);
		gluTessEndPolygon(tess);
	}
	gluDeleteTess(tess);

	// for (auto& pair : out_verts_for_mtl)
	//{
	//	std::vector<f32>& input_verts = pair.second;
	//	// indexify
	//	std::unordered_map<std::string, u32> vert2index;
	//	std::vector<f32> vertices;
	//	std::vector<u32> indices;
	//	u32 index = 0;
	//	for (u32 i = 0; i < input_verts.size(); i += VERT_SIZE)
	//	{
	//		printf("%u %zu\n", i, input_verts.size());
	//		const std::string& key = vert_to_string(input_verts.data() + i);
	//		if (!vert2index.contains(key))
	//		{
	//			vert2index.insert({ key, index });
	//			index++;
	//			for (u32 j = 0; j < VERT_SIZE; j++)
	//				vertices.push_back(input_verts[i + j]);
	//		}
	//		indices.push_back(vert2index.at(key));
	//	}
	//	printf("A\n%zu | %zu | %zu\n", input_verts.size(), (input_verts.size() / 5), (input_verts.size() / 5) % 3);
	//	printf("%zu | %zu\n", indices.size(), indices.size() % 3);
	//	// comp norms
	//	std::unordered_map<u32, vec<space::OBJECT>> norms;
	//	for (u32 i = 0; i < indices.size(); i += 3)
	//	{
	//		// indices
	//		const u32 a = indices[i + 0];
	//		const u32 b = indices[i + 1];
	//		const u32 c = indices[i + 2];
	//		// index into vertices array
	//		const u32 via = a * VERT_SIZE;
	//		const u32 vib = b * VERT_SIZE;
	//		const u32 vic = c * VERT_SIZE;
	//		// assumes xyz is first 3 elements
	//		const vec<space::OBJECT> va(vertices[via + 0], vertices[via + 1], vertices[via + 2]);
	//		const vec<space::OBJECT> vb(vertices[vib + 0], vertices[vib + 1], vertices[vib + 2]);
	//		const vec<space::OBJECT> vc(vertices[vic + 0], vertices[vic + 1], vertices[vic + 2]);
	//		const vec<space::OBJECT> ab = va - vb, ac = va - vc;
	//		// assumes ccw ordering (does glu force this or leave input winding?)
	//		const vec<space::OBJECT> normal = ab.cross_copy(ac);
	//		norms[a] += normal;
	//		norms[b] += normal;
	//		norms[c] += normal;
	//	}
	//	// average total normals
	//	for (auto& pair : norms)
	//		pair.second = pair.second.normalize_copy();
	//	const std::vector<f32> input_copy = input_verts;
	//	input_verts.clear();
	//	for (u32 i = 0; i < input_copy.size(); i += VERT_SIZE)
	//	{
	//		for (u32 j = 0; j < VERT_SIZE; j++)
	//			input_verts.push_back(input_copy[i + j]);
	//		const std::string& key = vert_to_string(input_copy.data() + i);
	//		const vec<space::OBJECT>& normal = norms.at(vert2index.at(key));
	//		input_verts.push_back(normal.x);
	//		input_verts.push_back(normal.y);
	//		input_verts.push_back(normal.z);
	//	}
	// }
}

void scene_ctx::m_draw_vaos(const mgl::context& glctx, const uniform_mats& mats, const std::unordered_map<u32, mgl::static_vertex_array>& vaos)
{
	for (auto it = vaos.begin(); it != vaos.end(); ++it)
	{
		const scene_material& mat = m_mtls[it->first];
		mat.shaders->bind();
		mat.shaders->uniform_mat4("u_mvp", mats.mvp.e);
		mat.shaders->uniform_mat4("u_mv", mats.mv.e);
		mat.shaders->uniform_mat4("u_m", mats.model.e);
		mat.shaders->uniform_mat4("u_normal", mats.normal.e);

		for (u32 i = 0; i < mat.texs.size(); i++)
		{
			mat.texs[i].second->bind(i);
			mat.shaders->uniform_1i(mat.texs[i].first.c_str(), i);
		}

		glctx.draw(it->second, *mat.shaders);
	}
}
