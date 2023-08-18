#include "pch.h"
#include "scene_ctx.h"
#include <pmp/surface_mesh.h>
#include <pmp/algorithms/normals.h>

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
			// mgl::static_vertex_array vao((f32*)it->second.data(), (u32)it->second.size() * s_vert_size, { 3, 2 });
			mgl::static_vertex_array vao((f32*)it->second.data(), (u32)it->second.size() * s_vert_size, { 3, 2, 3 });
			m_hm_vaos_for_mtl.emplace(it->first, std::move(vao));
		}

		m_hms_dirty = false;
	}
}

void scene_ctx::draw(const mgl::context& glctx, const scene_ctx_uniforms& mats)
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
		// mgl::static_vertex_array vao((f32*)it->second.data(), (u32)it->second.size() * s_vert_size, { 3, 2 });
		mgl::static_vertex_array vao((f32*)it->second.data(), (u32)it->second.size() * s_vert_size, { 3, 2, 3 });
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

		std::vector<tess_vtx> verts;
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
			verts.emplace_back(v);
		}

		gluTessBeginPolygon(tess, nullptr);
		gluTessBeginContour(tess);
		for (const tess_vtx& v : verts)
			gluTessVertex(tess, (GLdouble*)&v, (GLvoid*)&v);
		gluTessEndContour(tess);
		gluTessEndPolygon(tess);
	}
	gluDeleteTess(tess);

	pmp::SurfaceMesh normals_mesh;
	for (auto& pair : out_verts_for_mtl)
	{
		std::vector<mesh_vertex>& input_verts = pair.second;
		for (size_t i = 0; i < input_verts.size(); i += 3)
		{
			const auto& iv0 = input_verts[i];
			const auto& iv1 = input_verts[i + 1];
			const auto& iv2 = input_verts[i + 2];
			const auto v0 = normals_mesh.add_vertex(pmp::Point(iv0.x, iv0.y, iv0.z));
			const auto v1 = normals_mesh.add_vertex(pmp::Point(iv1.x, iv1.y, iv1.z));
			const auto v2 = normals_mesh.add_vertex(pmp::Point(iv2.x, iv2.y, iv2.z));
			normals_mesh.add_triangle(v0, v1, v2);
		}
	}
	pmp::vertex_normals(normals_mesh);
	for (const auto v : normals_mesh.vertices()) {
		const auto n = normals_mesh.get_vertex_property<pmp::Normal>("v:normal");
		std::cout << v << " -> " << *n.data() << "\n";
	}

	/*
	std::unordered_map<std::string, vec<space::OBJECT>> normals;
	for (const auto& pair : out_verts_for_mtl)
	{
		const auto& input_verts = pair.second;
		for (size_t i = 0; i < input_verts.size(); i += 3)
		{
			const auto& iv0 = input_verts[i];
			const auto& iv1 = input_verts[i + 1];
			const auto& iv2 = input_verts[i + 2];
			const vec<space::OBJECT> p0(iv0.x, iv0.y, iv0.z);
			const vec<space::OBJECT> p1(iv1.x, iv1.y, iv1.z);
			const vec<space::OBJECT> p2(iv2.x, iv2.y, iv2.z);
			const vec<space::OBJECT> p01 = p0 - p1, p02 = p0 - p2;
			const vec<space::OBJECT> norm = p01.cross_copy(p02);
			normals[iv0.to_string()] += norm;
			normals[iv1.to_string()] += norm;
			normals[iv2.to_string()] += norm;
		}
	}
	for (auto& pair : normals) {
		pair.second.normalize();
	}
	for (auto& pair : out_verts_for_mtl)
	{
		auto& input_verts = pair.second;
		for (size_t i = 0; i < input_verts.size(); i += 3)
		{
			auto& iv0 = input_verts[i];
			auto& iv1 = input_verts[i + 1];
			auto& iv2 = input_verts[i + 2];
			const auto& in0 = normals[iv0.to_string()];
			const auto& in1 = normals[iv1.to_string()];
			const auto& in2 = normals[iv2.to_string()];
			iv0.nx = in0.x;
			iv0.ny = in0.y;
			iv0.nz = in0.z;
			iv1.nx = in1.x;
			iv1.ny = in1.y;
			iv1.nz = in1.z;
			iv2.nx = in2.x;
			iv2.ny = in2.y;
			iv2.nz = in2.z;
		}
	}
	*/
}

void scene_ctx::m_draw_vaos(const mgl::context& glctx, const scene_ctx_uniforms& mats, const std::unordered_map<u32, mgl::static_vertex_array>& vaos)
{
	for (auto it = vaos.begin(); it != vaos.end(); ++it)
	{
		const scene_material& mat = m_mtls[it->first];
		mat.shaders->bind();
		mat.shaders->uniform_mat4("u_mvp", mats.mvp.e);
		mat.shaders->uniform_mat4("u_mv", mats.mv.e);
		mat.shaders->uniform_mat4("u_m", mats.model.e);
		mat.shaders->uniform_mat4("u_normal", mats.normal.e);
		mat.shaders->uniform_3fv("u_cam_pos", mats.cam_pos.e);


		for (u32 i = 0; i < mat.texs.size(); i++)
		{
			mat.texs[i].second->bind(i);
			mat.shaders->uniform_1i(mat.texs[i].first.c_str(), i);
		}

		glctx.draw(it->second, *mat.shaders);
	}
}
