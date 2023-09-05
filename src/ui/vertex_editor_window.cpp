#include "pch.h"
#include "vertex_editor_window.h"
#include "core/sgnode.h"
#include "app_ctx.h"
#include "geom/generated_mesh.h"

vertex_editor_window::vertex_editor_window(app_ctx* const app_ctx) :
	imgui_window(app_ctx, "Vertex Editor")
{}



void vertex_editor_window::handle_focused(const bool focused)
{
}
void vertex_editor_window::handle_frame()
{
	const sgnode* const selected_node = m_app_ctx->get_selected_sgnode();
	if (!(selected_node && selected_node->is_frozen()))
	{
		ImGui::TextDisabled("Select a Phrozen Node to Edit Vertices");
		return;
	}

	if (ImGui::BeginCombo("##VEW_COMBO", vertex_editor_mode_string(m_mode).c_str()))
	{
		for (s32 i = 0; i < (s32)vertex_editor_mode::COUNT; i++)
		{
			const vertex_editor_mode mode = (vertex_editor_mode)((s32)vertex_editor_mode::POSITION + i);
			const bool is_selected = (m_mode == mode);
			if (ImGui::Selectable(vertex_editor_mode_string(mode).c_str(), is_selected))
				m_mode = mode;
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	const mesh_t* mesh = selected_node->get_gen()->mesh;
	const auto& verts = mesh->vertex_storage;
	auto& vert_attrs = m_app_ctx->scene.get_vert_attrs();

	switch (m_mode)
	{
	case vertex_editor_mode::POSITION:
		{
			for (u64 i = 0; i < verts.size(); i++)
			{
				ImGui::Text("%d %f %f %f", i, verts[i].v.x, verts[i].v.y, verts[i].v.z);
			}
		}
		break;
	case vertex_editor_mode::COLOR:
		{
			std::unordered_map<const vertex_t*, u64> inserted;
			std::vector<std::vector<std::tuple<const face_t*, u32, u64>>> vec;
			u64 vert_id = 0;
			for (mesh_t::const_face_iter i = mesh->faceBegin(); i != mesh->faceEnd(); ++i)
			{
				const mesh_t::face_t* const f = *i;
				for (mesh_t::face_t::const_edge_iter_t e = f->begin(); e != f->end(); ++e)
				{
					const vertex_t* v = e->vert;
					if (inserted.contains(v))
					{
						vec.at(inserted.at(v)).push_back({ f, e.idx(), vert_id });
					}
					else
					{
						vec.push_back({ { f, e.idx(), vert_id } });
						inserted.insert({ v, vec.size() - 1 });
					}
					vert_id++;
				}
			}

			for (const auto& list : vec)
			{
				const color_t& color = vert_attrs.color.getAttribute(std::get<0>(list[0]), std::get<1>(list[0]));
				ImGui::Text("%d %f %f %f %f", std::get<2>(list[0]), color.r, color.g, color.b, color.a);
			}
		}
		break;
	case vertex_editor_mode::UV:
		{
			std::vector<std::pair<const face_t*, u32>> vec;
			for (mesh_t::const_face_iter i = mesh->faceBegin(); i != mesh->faceEnd(); ++i)
			{
				const mesh_t::face_t* const f = *i;
				for (mesh_t::face_t::const_edge_iter_t e = f->begin(); e != f->end(); ++e)
				{
					vec.push_back({ f, e.idx() });
				}
			}

			s32 i = 0;
			for (const auto& pair : vec)
			{
				const tex_coord_t& uv0 = vert_attrs.uv0.getAttribute(pair.first, pair.second);
				const tex_coord_t& uv1 = vert_attrs.uv1.getAttribute(pair.first, pair.second);
				const tex_coord_t& uv2 = vert_attrs.uv2.getAttribute(pair.first, pair.second);
				const tex_coord_t& uv3 = vert_attrs.uv3.getAttribute(pair.first, pair.second);
				ImGui::Text("%d (%f %f %f %f) (%f %f %f %f) (%f %f %f %f) (%f %f %f %f)", i++,
					uv0.u, uv0.v, uv0.uo, uv0.vo,
					uv1.u, uv1.v, uv1.uo, uv1.vo,
					uv2.u, uv2.v, uv2.uo, uv2.vo,
					uv3.u, uv3.v, uv3.uo, uv3.vo);
			}
		}
		break;
	}
}
