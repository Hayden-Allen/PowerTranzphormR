#include "pch.h"
#include "vertex_editor_window.h"
#include "core/sgnode.h"
#include "app_ctx.h"
#include "geom/generated_mesh.h"
#include "core/smnode.h"

vertex_editor_window::vertex_editor_window(app_ctx* const app_ctx) :
	imgui_window(app_ctx, "Vertex Editor")
{}



void vertex_editor_window::handle_focused(const bool focused)
{
}
void vertex_editor_window::handle_frame()
{
	sgnode* const selected_node = m_app_ctx->get_selected_sgnode();
	smnode* const selected_sm = m_app_ctx->get_selected_static_mesh();
	if (!(selected_node && selected_node->is_frozen() || (selected_sm && selected_sm->is_static())))
	{
		ImGui::TextDisabled("Select a Phrozen Node to Edit Vertices");
		return;
	}
	if (selected_node)
	{
		if (handle_frame_base(selected_node->get_gen()->mesh, selected_node->get_local_verts(), selected_node->accumulate_mats(), selected_node->get_gen()->get_material()))
			selected_node->set_dirty();
	}
	else
	{
		if (handle_frame_base(selected_sm->get_mesh(), selected_sm->get_local_verts(), selected_sm->get_mat(), selected_sm->get_material()))
			selected_sm->set_gen_dirty();
	}
}



bool vertex_editor_window::handle_frame_base(const mesh_t* const mesh, std::vector<point<space::OBJECT>>& local_verts, const tmat<space::OBJECT, space::WORLD>& mat, const GLuint mtl_id)
{
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

	auto& vert_attrs = m_app_ctx->scene.get_vert_attrs();
	const f32 item_spacing_x = ImGui::GetStyle().ItemSpacing.x;
	const f32 half_item_spacing_x = item_spacing_x * 0.5f;
	const s32 col_vert_id_width = 56;
	const f32 col_position_width = 300.0f, col_color_width = 300.0f, col_uv_width = 300.0f, col_weight_width = 300.0f;
	bool found_activated = false;
	bool changed = false;

	switch (m_mode)
	{
	case vertex_editor_mode::POSITION:
		{
			if (ImGui::BeginTable("vew_pos", 2))
			{
				ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, col_vert_id_width);
				ImGui::TableSetupColumn("Position", ImGuiTableColumnFlags_WidthFixed, col_position_width);
				const f32 drag_float_width = (col_position_width - half_item_spacing_x * 2.0f) / 3.0f;

				for (s32 vert_i = 0; vert_i < (s32)local_verts.size(); ++vert_i)
				{
					f32 editable_pos[3] = { local_verts[vert_i].x, local_verts[vert_i].y, local_verts[vert_i].z };
					bool pos_dirty = false;

					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					ImGui::PushID(vert_i);

					ImGui::PushID("id");
					ImGui::Text("%d", vert_i);
					ImGui::PopID();

					ImGui::TableNextColumn();

					ImGui::BeginGroup();

					for (s32 comp = 0; comp < 3; comp++)
					{
						if (comp > 0)
						{
							ImGui::SameLine();
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() - half_item_spacing_x);
						}

						ImGui::PushID(comp);

						ImGui::SetNextItemWidth(drag_float_width);
						if (ImGui::DragFloat("", &editable_pos[comp], 0.01f))
						{
							pos_dirty = true;
						}

						ImGui::PopID();
					}

					ImGui::EndGroup();
					if (!found_activated && (ImGui::IsItemHovered() || ImGui::IsItemActive()))
					{
						m_app_ctx->set_vertex_editor_icon_position(local_verts[vert_i].transform_copy(mat), vert_i);
						if (ImGui::IsItemActive())
							found_activated = true;
					}

					if (pos_dirty)
					{
						local_verts[vert_i].x = editable_pos[0];
						local_verts[vert_i].y = editable_pos[1];
						local_verts[vert_i].z = editable_pos[2];
						changed = true;
					}

					ImGui::PopID();
				}

				ImGui::EndTable();
			}
		}
		break;
	case vertex_editor_mode::COLOR:
		{
			std::unordered_map<const vertex_t*, u64> inserted;
			std::vector<std::vector<std::tuple<const face_t*, u32, const vertex_t*>>> vec;
			u64 vert_id = 0;
			for (mesh_t::const_face_iter i = mesh->faceBegin(); i != mesh->faceEnd(); ++i)
			{
				const mesh_t::face_t* const f = *i;
				for (mesh_t::face_t::const_edge_iter_t e = f->begin(); e != f->end(); ++e)
				{
					const vertex_t* v = e->vert;
					if (inserted.contains(v))
					{
						vec.at(inserted.at(v)).push_back({ f, e.idx(), v });
					}
					else
					{
						vec.push_back({ { f, e.idx(), v } });
						inserted.insert({ v, vec.size() - 1 });
					}
				}
			}


			if (ImGui::BeginTable("vew_col", 2))
			{
				ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, col_vert_id_width);
				ImGui::TableSetupColumn("Color", ImGuiTableColumnFlags_WidthFixed, col_color_width);
				const f32 drag_float_width = (col_uv_width - half_item_spacing_x * 3.0f) * 0.25f;

				for (s32 vert_i = 0; vert_i < (s32)vec.size(); ++vert_i)
				{
					const auto& list = vec[vert_i];
					const color_t& color = vert_attrs.color.getAttribute(std::get<0>(list[0]), std::get<1>(list[0]));
					f32 editable_color[4] = { color.r, color.g, color.b, color.a };
					bool color_dirty = false;

					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					ImGui::PushID(vert_i);

					ImGui::PushID("id");
					ImGui::Text("%d", vert_i);
					ImGui::PopID();

					ImGui::TableNextColumn();

					ImGui::BeginGroup();

					for (s32 comp = 0; comp < 4; comp++)
					{
						if (comp > 0)
						{
							ImGui::SameLine();
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() - half_item_spacing_x);
						}

						ImGui::PushID(comp);

						ImGui::SetNextItemWidth(drag_float_width);
						if (ImGui::DragFloat("", &editable_color[comp], 0.01f, 0.0f, 1.0f))
						{
							color_dirty = true;
						}

						ImGui::PopID();
					}

					ImGui::EndGroup();
					if (!found_activated && (ImGui::IsItemHovered() || ImGui::IsItemActive()))
					{
						const vertex_t* v = std::get<2>(list[0]);
						m_app_ctx->set_vertex_editor_icon_position(point<space::WORLD>(v->v.x, v->v.y, v->v.z), vert_i);
						if (ImGui::IsItemActive())
							found_activated = true;
					}

					if (color_dirty)
					{
						for (const auto& tuple : list)
						{
							vert_attrs.color.setAttribute(std::get<0>(tuple), std::get<1>(tuple), color_t(editable_color[0], editable_color[1], editable_color[2], editable_color[3]));
						}
						changed = true;
					}

					ImGui::PopID();
				}

				ImGui::EndTable();
			}
		}
		break;
	case vertex_editor_mode::UV:
		{
			std::vector<std::tuple<const face_t*, u32, const vertex_t*>> vec;
			for (mesh_t::const_face_iter i = mesh->faceBegin(); i != mesh->faceEnd(); ++i)
			{
				const mesh_t::face_t* const f = *i;
				for (mesh_t::face_t::const_edge_iter_t e = f->begin(); e != f->end(); ++e)
				{
					vec.push_back({ f, e.idx(), e->vert });
				}
			}

			if (ImGui::BeginTable("vew_uv", 5))
			{
				ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, col_vert_id_width);
				ImGui::TableSetupColumn("Tex0", ImGuiTableColumnFlags_WidthFixed, col_uv_width);
				ImGui::TableSetupColumn("Tex1", ImGuiTableColumnFlags_WidthFixed, col_uv_width);
				ImGui::TableSetupColumn("Tex2", ImGuiTableColumnFlags_WidthFixed, col_uv_width);
				ImGui::TableSetupColumn("Tex3", ImGuiTableColumnFlags_WidthFixed, col_uv_width);
				const f32 drag_float_width = (col_uv_width - half_item_spacing_x * 3.0f) * 0.25f;

				for (s32 vert_i = 0; vert_i < (s32)vec.size(); ++vert_i)
				{
					const auto& tuple = vec[vert_i];
					const tex_coord_t& uv0 = vert_attrs.uv0.getAttribute(std::get<0>(tuple), std::get<1>(tuple));
					const tex_coord_t& uv1 = vert_attrs.uv1.getAttribute(std::get<0>(tuple), std::get<1>(tuple));
					const tex_coord_t& uv2 = vert_attrs.uv2.getAttribute(std::get<0>(tuple), std::get<1>(tuple));
					const tex_coord_t& uv3 = vert_attrs.uv3.getAttribute(std::get<0>(tuple), std::get<1>(tuple));
					f32 editable_uvw[4][4] = {
						{ uv0.u, uv0.v, uv0.uo, uv0.vo },
						{ uv1.u, uv1.v, uv1.uo, uv1.vo },
						{ uv2.u, uv2.v, uv2.uo, uv2.vo },
						{ uv3.u, uv3.v, uv3.uo, uv3.vo },
					};

					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					ImGui::PushID(vert_i);

					ImGui::PushID("id");
					ImGui::Text("%d", vert_i);
					ImGui::PopID();

					for (s32 chan = 0; chan < 4; ++chan)
					{
						bool chan_dirty = false;

						ImGui::TableNextColumn();
						ImGui::PushID(chan);

						ImGui::BeginGroup();

						for (s32 comp = 0; comp < 4; ++comp)
						{
							if (comp > 0)
							{
								ImGui::SameLine();
								ImGui::SetCursorPosX(ImGui::GetCursorPosX() - half_item_spacing_x);
							}

							ImGui::PushID(comp);

							ImGui::SetNextItemWidth(drag_float_width);
							if (ImGui::DragFloat("", &editable_uvw[chan][comp], 0.01f, 0.0f, comp == 4 ? 1.0f : 0.0f))
							{
								chan_dirty = true;
							}

							ImGui::PopID();
						}

						ImGui::EndGroup();
						if (!found_activated && (ImGui::IsItemHovered() || ImGui::IsItemActive()))
						{
							const vertex_t* v = std::get<2>(tuple);
							m_app_ctx->set_vertex_editor_icon_position(point<space::WORLD>(v->v.x, v->v.y, v->v.z), vert_i);
							if (ImGui::IsItemActive())
								found_activated = true;
						}

						ImGui::PopID();

						if (chan_dirty)
						{
							tex_coord_t new_tc(editable_uvw[chan][0], editable_uvw[chan][1], editable_uvw[chan][2], editable_uvw[chan][3]);
							switch (chan)
							{
							case 0:
								vert_attrs.uv0.setAttribute(std::get<0>(tuple), std::get<1>(tuple), new_tc);
								break;
							case 1:
								vert_attrs.uv1.setAttribute(std::get<0>(tuple), std::get<1>(tuple), new_tc);
								break;
							case 2:
								vert_attrs.uv2.setAttribute(std::get<0>(tuple), std::get<1>(tuple), new_tc);
								break;
							case 3:
								vert_attrs.uv3.setAttribute(std::get<0>(tuple), std::get<1>(tuple), new_tc);
								break;
							default:
								assert(false);
								break;
							}
							changed = true;
						}
					}

					ImGui::PopID();
				}

				ImGui::EndTable();
			}
		}
		break;
	case vertex_editor_mode::TEX_WEIGHT:
		{
			std::vector<std::vector<std::tuple<const face_t*, u32, const vertex_t*>>> vec;
			if (mtl_id != MAX_VALUE(mtl_id))
			{
				std::unordered_map<const vertex_t*, u64> inserted;
				u64 vert_id = 0;
				for (mesh_t::const_face_iter i = mesh->faceBegin(); i != mesh->faceEnd(); ++i)
				{
					const mesh_t::face_t* const f = *i;
					for (mesh_t::face_t::const_edge_iter_t e = f->begin(); e != f->end(); ++e)
					{
						const vertex_t* v = e->vert;
						if (inserted.contains(v))
						{
							vec.at(inserted.at(v)).push_back({ f, e.idx(), v });
						}
						else
						{
							vec.push_back({ { f, e.idx(), v } });
							inserted.insert({ v, vec.size() - 1 });
						}
					}
				}
			}
			else
			{
				for (mesh_t::const_face_iter i = mesh->faceBegin(); i != mesh->faceEnd(); ++i)
				{
					const mesh_t::face_t* const f = *i;
					for (mesh_t::face_t::const_edge_iter_t e = f->begin(); e != f->end(); ++e)
					{
						vec.push_back({ { f, e.idx(), e->vert } });
					}
				}
			}


			if (ImGui::BeginTable("vew_weight", 2))
			{
				ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, col_vert_id_width);
				ImGui::TableSetupColumn("Weights", ImGuiTableColumnFlags_WidthFixed, col_weight_width);
				const f32 drag_float_width = (col_weight_width - half_item_spacing_x * 3.0f) * 0.25f;

				for (s32 vert_i = 0; vert_i < (s32)vec.size(); ++vert_i)
				{
					const auto& list = vec[vert_i];
					const f32& w0 = (f32)vert_attrs.w0.getAttribute(std::get<0>(list[0]), std::get<1>(list[0]));
					const f32& w1 = (f32)vert_attrs.w1.getAttribute(std::get<0>(list[0]), std::get<1>(list[0]));
					const f32& w2 = (f32)vert_attrs.w2.getAttribute(std::get<0>(list[0]), std::get<1>(list[0]));
					const f32& w3 = (f32)vert_attrs.w3.getAttribute(std::get<0>(list[0]), std::get<1>(list[0]));
					f32 editable_weights[4] = { w0, w1, w2, w3 };
					bool weights_dirty = false;

					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					ImGui::PushID(vert_i);

					ImGui::PushID("id");
					ImGui::Text("%d", vert_i);
					ImGui::PopID();

					ImGui::TableNextColumn();

					ImGui::BeginGroup();

					for (s32 comp = 0; comp < 4; comp++)
					{
						if (comp > 0)
						{
							ImGui::SameLine();
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() - half_item_spacing_x);
						}

						ImGui::PushID(comp);

						ImGui::SetNextItemWidth(drag_float_width);
						if (ImGui::DragFloat("", &editable_weights[comp], 0.01f, 0.0f, 1.0f))
						{
							weights_dirty = true;
						}

						ImGui::PopID();
					}

					ImGui::EndGroup();
					if (!found_activated && (ImGui::IsItemHovered() || ImGui::IsItemActive()))
					{
						const vertex_t* v = std::get<2>(list[0]);
						m_app_ctx->set_vertex_editor_icon_position(point<space::WORLD>(v->v.x, v->v.y, v->v.z), vert_i);
						if (ImGui::IsItemActive())
							found_activated = true;
					}

					if (weights_dirty)
					{
						for (const auto& tuple : list)
						{
							vert_attrs.w0.setAttribute(std::get<0>(tuple), std::get<1>(tuple), editable_weights[0]);
							vert_attrs.w1.setAttribute(std::get<0>(tuple), std::get<1>(tuple), editable_weights[1]);
							vert_attrs.w2.setAttribute(std::get<0>(tuple), std::get<1>(tuple), editable_weights[2]);
							vert_attrs.w3.setAttribute(std::get<0>(tuple), std::get<1>(tuple), editable_weights[3]);
						}
						changed = true;
					}

					ImGui::PopID();
				}

				ImGui::EndTable();
			}
		}
		break;
	}

	m_app_ctx->check_vertex_editor_icon_switched();
	return changed;
}
