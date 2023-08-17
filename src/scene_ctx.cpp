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

	for (auto& pair : out_verts_for_mtl)
	{
		const std::vector<mesh_vertex>& input_verts = pair.second;
		// index
		std::unordered_map<std::string, u32> vert2index;
		// std::unordered_map<std::string, std::unordered_map<u32, std::vector<direction<space::OBJECT>>>> vert2index;
		std::vector<mesh_vertex> unique_verts;
		std::vector<u32> indices;
		u32 index_count = 0;
		for (u32 i = 0; i < input_verts.size(); i++)
		{
			const mesh_vertex& mv = input_verts[i];
			const std::string& key = mv.to_string();
			if (!vert2index.contains(key))
			{
				vert2index.insert({ key, index_count });
				index_count++;
				unique_verts.push_back(mv);
			}
			indices.push_back(vert2index.at(key));
		}
		/*
		assert(input_verts.size() % 3 == 0);
		for (u32 i = 0; i < input_verts.size(); i += 3)
		{
			// printf("%u\n", i);
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
				// printf("j %u\n", j);
				const mesh_vertex& mv = input_verts[i + j];
				const std::string& key = mv.to_string();
				const auto& it = vert2index.find(key);
				// this vertex has been seen before, try to match it to existing instance
				bool found = false;
				if (it != vert2index.end())
				{
					// check existing instances of this vertex
					for (auto& faces : it->second)
					{
						// printf("%zu\n", faces.second.size());
						// check all faces that each instance is part of
						for (u32 k = 0; k < faces.second.size(); k++)
						{
							// printf("k %u\n", k);
							const auto& face_norm = faces.second.at(k);
							const f32 angle = norm.angle_to(face_norm);
							// current vertex can not be added to this instance
							if (angle >= s_snap_angle)
								break;
							// made it to the end of the list, current vertex is part of this instance
							if (k == faces.second.size() - 1)
							{
								faces.second.push_back(norm);
								indices.push_back(faces.first);
								found = true;
								break;
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
					// printf("NEW\n");
					vert2index.insert({
						key,
						{ { index_count, { norm } } },
					});
					indices.push_back(index_count);
					index_count++;
					unique_verts.push_back(mv);
				}
			}
		}
		*/
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
			const vec<space::OBJECT> pa(va.x, va.y, va.z);
			const vec<space::OBJECT> pb(vb.x, vb.y, vb.z);
			const vec<space::OBJECT> pc(vc.x, vc.y, vc.z);
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
		pair.second.clear();
		for (const u32 i : indices)
		{
			vec<space::OBJECT>& norm = norms[i];
			// norm.print();
			unique_verts[i].nx = norm.x;
			unique_verts[i].ny = norm.y;
			unique_verts[i].nz = norm.z;
			pair.second.push_back(unique_verts[i]);
		}
	}
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
