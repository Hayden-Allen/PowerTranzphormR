#include "pch.h"
#include "scene_graph_window.h"
#include "app_ctx.h"
#include "core/sgnode.h"
#include "core/smnode.h"
#include "geom/generated_mesh.h"

scene_graph_window::scene_graph_window(app_ctx* const a_ctx) :
	imgui_window(a_ctx, "Phorms")
{}



void scene_graph_window::handle_focused(const bool focused)
{
	if (focused)
	{
		if (!m_was_focused)
		{
			m_app_ctx->set_sel_type(m_prev_sel_type);
		}
		m_was_focused = true;
	}
	else
	{
		m_was_focused = false;
	}
}
void scene_graph_window::handle_frame()
{
	bool performed_destructive_action = false;

	sgnode* const root = m_app_ctx->scene.get_sg_root();
	ImGui::PushID("hfsgn");
	handle_node(root, performed_destructive_action);
	ImGui::PopID();
	if (!performed_destructive_action)
	{
		m_app_ctx->unset_imgui_needs_select_unfocused_sgnode();
	}

	performed_destructive_action = false;
	ImGui::PushID("hfhmp");
	handle_heightmaps(performed_destructive_action);
	ImGui::PopID();
	if (!performed_destructive_action)
	{
		m_app_ctx->unset_imgui_needs_select_unfocused_static_mesh();
	}

	performed_destructive_action = false;
	ImGui::PushID("hflit");
	handle_lights(performed_destructive_action);
	ImGui::PopID();
	if (!performed_destructive_action)
	{
		m_app_ctx->unset_imgui_needs_select_unfocused_light();
	}

	performed_destructive_action = false;
	ImGui::PushID("hfway");
	handle_waypoints(performed_destructive_action);
	ImGui::PopID();
	if (!performed_destructive_action)
	{
		m_app_ctx->unset_imgui_needs_select_unfocused_waypoint();
	}

	if (m_app_ctx->sel_type != global_selection_type::material)
	{
		m_prev_sel_type = m_app_ctx->sel_type;
	}
}
void scene_graph_window::set_renaming(sgnode* const node)
{
	assert(!m_renaming);
	m_renaming = node;
	m_rename_needs_focus = true;
}
const sgnode* scene_graph_window::get_renaming() const
{
	return m_renaming;
}
void scene_graph_window::set_renaming_sm(smnode* const sm)
{
	assert(!m_renaming_sm);
	m_renaming_sm = sm;
	m_rename_sm_needs_focus = true;
}
const smnode* scene_graph_window::get_renaming_sm() const
{
	return m_renaming_sm;
}
void scene_graph_window::set_renaming_light(light* const l)
{
	assert(!m_renaming_light);
	m_renaming_light = l;
	m_rename_light_needs_focus = true;
}
const light* scene_graph_window::get_renaming_light() const
{
	return m_renaming_light;
}
void scene_graph_window::set_renaming_waypoint(waypoint* const w)
{
	assert(!m_renaming_waypoint);
	m_renaming_waypoint = w;
	m_rename_waypoint_needs_focus = true;
}
const waypoint* scene_graph_window::get_renaming_waypoint() const
{
	return m_renaming_waypoint;
}
scene_graph_window::rect scene_graph_window::handle_node(sgnode* const node, bool& performed_destructive_action)
{
	// draw current node
	std::string display_name = node->get_name();
	const std::string op_name = u::operation_to_string(node->get_operation());
	if (node->is_root())
	{
		display_name = "Scene Graph";
	}
	else if (node->is_operation() && display_name != op_name)
	{
		display_name += " [" + op_name + "]";
	}

	const f32 padding_x = 3.f;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(padding_x, 2.f));
	bool open = false;
	if (node == m_renaming)
	{
		const f32 x = ImGui::GetCursorPosX();

		bool is_sel_or_multisel = (m_app_ctx->get_selected_sgnode() == node) || m_app_ctx->is_sgnode_multiselected(node);
		if (!node->is_visible())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		}
		open = ImGui::TreeNodeEx(node->get_id().c_str(), ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth | (is_sel_or_multisel ? ImGuiTreeNodeFlags_Selected : 0) | (node->get_children().size() == 0 ? ImGuiTreeNodeFlags_Leaf : 0), "%s", display_name.c_str());
		if (!node->is_visible())
		{
			ImGui::PopStyleColor();
		}

		constexpr u32 BUF_SIZE = 32;
		char buf[32] = { 0 };
		memcpy_s(buf, BUF_SIZE, node->get_name().c_str(), node->get_name().size());

		// FIXME kind of hacky?
		ImGui::SameLine();
		const f32 size = ImGui::GetFontSize();
		ImGui::SetCursorPosX(x + size + padding_x);

		if (ImGui::InputText("##SGW_RENAME", buf, BUF_SIZE, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			std::string new_name(buf);
			if (!new_name.empty())
			{
				m_app_ctx->rename_action(node, new_name);
			}
			m_renaming = nullptr;
		}

		if (m_rename_needs_focus)
		{
			ImGui::SetKeyboardFocusHere(-1);
			m_rename_needs_focus = false;
		}
		else if (!ImGui::IsItemActive())
		{
			m_renaming = nullptr;
		}
	}
	else
	{
		bool is_sel_or_multisel = (m_app_ctx->get_selected_sgnode() == node) || m_app_ctx->is_sgnode_multiselected(node);
		if (!node->is_visible())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		}
		open = ImGui::TreeNodeEx(node->get_id().c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth | (is_sel_or_multisel ? ImGuiTreeNodeFlags_Selected : 0) | (node->get_children().size() == 0 ? ImGuiTreeNodeFlags_Leaf : 0), "%s", display_name.c_str());
		if (!node->is_visible())
		{
			ImGui::PopStyleColor();
		}
	}



	sgnode* needs_select = m_app_ctx->get_imgui_needs_select_unfocused_sgnode();
	if (needs_select)
	{
		if (node == needs_select)
		{
			ImGui::SetKeyboardFocusHere(-1);
			m_app_ctx->set_selected_sgnode(node);
		}
	}
	else if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl) || ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift))
	{
		if (!node->is_root())
		{
			if (ImGui::IsItemClicked())
			{
				m_app_ctx->toggle_sgnode_multiselected(node);
			}
			else if (ImGui::IsItemFocused() && m_app_ctx->get_num_multiselected_sgnodes() > 0 && !m_app_ctx->is_sgnode_multiselected(node))
			{
				m_app_ctx->toggle_sgnode_multiselected(node);
			}
		}
	}
	else
	{
		if (ImGui::IsItemClicked() || (ImGui::IsItemFocused() && !m_app_ctx->is_sgnode_multiselected(node)))
		{
			m_app_ctx->set_selected_sgnode(node);
		}
	}

	ImGui::PopStyleVar();
	const ImVec2& cur_min = ImGui::GetItemRectMin();
	const ImVec2& cur_max = ImGui::GetItemRectMax();

	bool group_happened = false;
	// handle controls
	ImGui::PushID(node->get_id().c_str());
	if (ImGui::BeginPopupContextItem())
	{
		if (m_app_ctx->get_num_multiselected_sgnodes() <= 1 || !m_app_ctx->is_sgnode_multiselected(node))
		{
			m_app_ctx->set_selected_sgnode(node);

			if (node->is_operation())
			{
				if (ImGui::MenuItem("Add Cube"))
				{
					m_app_ctx->create_cube_action();
					performed_destructive_action = true;
				}
				if (ImGui::MenuItem("Add Sphere"))
				{
					m_app_ctx->create_sphere_action();
					performed_destructive_action = true;
				}
				if (ImGui::MenuItem("Add Cylinder"))
				{
					m_app_ctx->create_cylinder_action();
					performed_destructive_action = true;
				}
				if (ImGui::MenuItem("Add Cone"))
				{
					m_app_ctx->create_cone_action();
					performed_destructive_action = true;
				}
				if (ImGui::MenuItem("Add Torus"))
				{
					m_app_ctx->create_torus_action();
					performed_destructive_action = true;
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Add Union"))
				{
					m_app_ctx->create_union_action();
					performed_destructive_action = true;
				}
				if (ImGui::MenuItem("Add Subtract"))
				{
					m_app_ctx->create_subtract_action();
					performed_destructive_action = true;
				}
				if (ImGui::MenuItem("Add Intersect"))
				{
					m_app_ctx->create_intersect_action();
					performed_destructive_action = true;
				}

				if (!node->is_root())
				{
					ImGui::Separator();
					const auto& op = node->get_operation();
					if (op != carve::csg::CSG::OP::UNION && ImGui::MenuItem("Make Union"))
					{
						m_app_ctx->set_operation_action(node, carve::csg::CSG::OP::UNION);
					}
					if (op != carve::csg::CSG::OP::A_MINUS_B && ImGui::MenuItem("Make Subtract"))
					{
						m_app_ctx->set_operation_action(node, carve::csg::CSG::OP::A_MINUS_B);
					}
					if (op != carve::csg::CSG::OP::INTERSECTION && ImGui::MenuItem("Make Intersection"))
					{
						m_app_ctx->set_operation_action(node, carve::csg::CSG::OP::INTERSECTION);
					}
				}
			}

			if (!node->is_root())
			{
				if (node->is_operation())
					ImGui::Separator();
				if (node->is_visible())
				{
					if (ImGui::MenuItem("Hide"))
						node->set_visibility(false);
				}
				else
				{
					if (ImGui::MenuItem("Show"))
						node->set_visibility(true);
				}
				if (!node->is_operation())
					ImGui::Separator();
			}

			if (!node->is_root() && node->get_gen()->mesh)
			{
				if (node->is_operation())
					ImGui::Separator();
				// if the current node is the original frozen version of a subtree
				const bool has_unfrozen = m_app_ctx->has_unfrozen(node);
				if (has_unfrozen)
				{
					if (ImGui::MenuItem("Unphreeze!"))
					{
						m_app_ctx->unfreeze_action(node);
						performed_destructive_action = true;
					}
				}
				// if the current node is NOT a clone of an original frozen node
				else if (!node->is_frozen())
				{
					if (ImGui::MenuItem("Phreeze!"))
					{
						m_app_ctx->freeze_action(node);
						performed_destructive_action = true;
					}
				}
				if (node->is_frozen() && ImGui::MenuItem("Clone to Static Mesh"))
				{
					m_app_ctx->make_sgnode_static(node);
				}
			}
			if (!node->is_root())
			{
				ImGui::Separator();
				if (ImGui::MenuItem("Rename"))
				{
					set_renaming(node);
				}
			}
			if (!node->is_root() && ImGui::MenuItem("Destroy"))
			{
				m_app_ctx->destroy_selected_action();
				performed_destructive_action = true;
			}
		}
		else
		{
			if (ImGui::MenuItem("Group to Union"))
			{
				m_app_ctx->group_to_operation_action(carve::csg::CSG::OP::UNION);
				performed_destructive_action = true;
			}
			if (ImGui::MenuItem("Group to Subtract"))
			{
				m_app_ctx->group_to_operation_action(carve::csg::CSG::OP::A_MINUS_B);
				performed_destructive_action = true;
			}
			if (ImGui::MenuItem("Group to Intersect"))
			{
				m_app_ctx->group_to_operation_action(carve::csg::CSG::OP::INTERSECTION);
				performed_destructive_action = true;
			}
		}

		ImGui::EndPopup();
	}
	ImGui::PopID();

	if (open)
	{
		// draw children if it has any
		if (!performed_destructive_action && node->get_children().size())
		{
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			const ImColor line_color(128, 128, 128);
			// start vertical line at position of current node
			ImVec2 vertical_start = ImGui::GetCursorScreenPos();
			vertical_start.x -= 8.f;
			vertical_start.y -= 8.f;
			ImVec2 vertical_end = vertical_start;
			for (sgnode* const child : node->get_children())
			{
				bool child_performed_destructive_action = false;
				const rect& child_rect = handle_node(child, child_performed_destructive_action);
				if (child_performed_destructive_action)
				{
					performed_destructive_action = true;
					break;
				}
				// draw horizontal line from vertical line to current child
				const f32 horizontal_size = child->get_children().size() > 0 ? 12.f : 24.f;
				const f32 midpoint = (child_rect.first.y + child_rect.second.y) / 2.f;
				draw_list->AddLine(ImVec2(vertical_start.x, midpoint), ImVec2(vertical_start.x + horizontal_size, midpoint), line_color);
				// end vertical line at position of current child
				vertical_end.y = midpoint;
			}
			// draw vertical line
			draw_list->AddLine(vertical_start, vertical_end, line_color);
		}

		ImGui::TreePop();
	}

	return std::make_pair(cur_min, cur_max);
}
void scene_graph_window::handle_heightmap(smnode* const hmp, bool& performed_destructive_action)
{
	const f32 padding_x = 3.f;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(padding_x, 2.f));
	if (hmp == m_renaming_sm)
	{
		const f32 x = ImGui::GetCursorPosX();

		if (!hmp->is_visible())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		}
		ImGui::TreeNodeEx(hmp->get_id().c_str(), ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth | (hmp == m_app_ctx->get_selected_static_mesh() ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_Leaf, "%s", hmp->get_name().c_str());
		if (!hmp->is_visible())
		{
			ImGui::PopStyleColor();
		}

		constexpr u32 BUF_SIZE = 32;
		char buf[32] = { 0 };
		memcpy_s(buf, BUF_SIZE, hmp->get_name().c_str(), hmp->get_name().size());

		// FIXME kind of hacky?
		ImGui::SameLine();
		const f32 size = ImGui::GetFontSize();
		ImGui::SetCursorPosX(x + size + padding_x);

		if (ImGui::InputText("##SGW_RENAME_SM", buf, BUF_SIZE, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			std::string new_name(buf);
			if (!new_name.empty())
			{
				hmp->set_name(new_name);
			}
			m_renaming_sm = nullptr;
		}

		if (m_rename_sm_needs_focus)
		{
			ImGui::SetKeyboardFocusHere(-1);
			m_rename_sm_needs_focus = false;
		}
		else if (!ImGui::IsItemActive())
		{
			m_renaming_sm = nullptr;
		}
	}
	else
	{
		if (!hmp->is_visible())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		}
		ImGui::TreeNodeEx(hmp->get_id().c_str(), ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth | (hmp == m_app_ctx->get_selected_static_mesh() ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_Leaf, "%s", hmp->get_name().c_str());
		if (!hmp->is_visible())
		{
			ImGui::PopStyleColor();
		}
	}

	smnode* needs_select = m_app_ctx->get_imgui_needs_select_unfocused_static_mesh();
	if (needs_select)
	{
		if (hmp == needs_select)
		{
			ImGui::SetKeyboardFocusHere(-1);
		}
	}
	else if (ImGui::IsItemFocused())
	{
		m_app_ctx->set_selected_static_mesh(hmp);
	}

	ImGui::PopStyleVar();
	const ImVec2& cur_min = ImGui::GetItemRectMin();
	const ImVec2& cur_max = ImGui::GetItemRectMax();

	// handle controls
	ImGui::PushID(hmp->get_id().c_str());
	if (ImGui::BeginPopupContextItem())
	{
		m_app_ctx->set_selected_static_mesh(hmp);

		if (hmp->is_visible())
		{
			if (ImGui::MenuItem("Hide"))
				hmp->set_visibility(false);
		}
		else
		{
			if (ImGui::MenuItem("Show"))
				hmp->set_visibility(true);
		}
		ImGui::Separator();

		if (!hmp->is_static())
		{
			if (ImGui::MenuItem("Phreeze!"))
			{
				hmp->make_static(&m_app_ctx->scene);
				hmp->set_name("Phrozen " + hmp->get_name());
				performed_destructive_action = true;
			}
		}
		else
		{
			if (ImGui::MenuItem("Clone to Scene Graph"))
			{
				m_app_ctx->make_frozen_sgnode_from_smnode(hmp);
			}
		}
		ImGui::Separator();

		if (ImGui::MenuItem("Duplicate"))
		{
			m_app_ctx->duplicate_selected_static_mesh();
			performed_destructive_action = true;
		}
		ImGui::Separator();

		if (ImGui::MenuItem("Rename"))
		{
			set_renaming_sm(hmp);
		}
		if (ImGui::MenuItem("Destroy"))
		{
			m_app_ctx->destroy_static_mesh(hmp);
			performed_destructive_action = true;
		}
		ImGui::EndPopup();
	}
	ImGui::PopID();

	ImGui::TreePop();
}
void scene_graph_window::handle_heightmaps(bool& performed_destructive_action)
{
	const f32 padding_x = 3.f;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(padding_x, 2.f));
	const bool open = ImGui::TreeNodeEx("##SGW_HMS", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth, "Static Meshes");
	ImGui::PopStyleVar();

	ImGui::PushID("##SGW_HMS");
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::MenuItem("Add Heightmap"))
		{
			m_app_ctx->create_heightmap();
		}
		ImGui::EndPopup();
	}
	ImGui::PopID();

	if (open)
	{
		auto& sms = m_app_ctx->scene.get_static_meshes();
		for (u32 i = 0; i < sms.size(); i++)
		{
			handle_heightmap(sms[i], performed_destructive_action);
			if (performed_destructive_action)
			{
				break;
			}
		}
		ImGui::TreePop();
	}
}
void scene_graph_window::handle_light(const u32 index, bool& performed_destructive_action)
{
	light* const l = m_app_ctx->scene.get_lights()[index];
	const f32 padding_x = 3.f;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(padding_x, 2.f));
	if (l == m_renaming_light)
	{
		const f32 x = ImGui::GetCursorPosX();

		if (!l->is_visible())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		}
		ImGui::TreeNodeEx(l->get_id().c_str(), ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth | (l == m_app_ctx->get_selected_light() ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_Leaf, "%s", l->get_name().c_str());
		if (!l->is_visible())
		{
			ImGui::PopStyleColor();
		}

		constexpr u32 BUF_SIZE = 32;
		char buf[32] = { 0 };
		memcpy_s(buf, BUF_SIZE, l->get_name().c_str(), l->get_name().size());

		// FIXME kind of hacky?
		ImGui::SameLine();
		const f32 size = ImGui::GetFontSize();
		ImGui::SetCursorPosX(x + size + padding_x);

		if (ImGui::InputText("##SGW_RENAME_LIT", buf, BUF_SIZE, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			std::string new_name(buf);
			if (!new_name.empty())
			{
				l->set_name(new_name);
			}
			m_renaming_light = nullptr;
		}

		if (m_rename_light_needs_focus)
		{
			ImGui::SetKeyboardFocusHere(-1);
			m_rename_light_needs_focus = false;
		}
		else if (!ImGui::IsItemActive())
		{
			m_renaming_light = nullptr;
		}
	}
	else
	{
		if (!l->is_visible())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		}
		ImGui::TreeNodeEx(l->get_id().c_str(), ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth | (l == m_app_ctx->get_selected_light() ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_Leaf, "%s", l->get_name().c_str());
		if (!l->is_visible())
		{
			ImGui::PopStyleColor();
		}
	}

	light* needs_select = m_app_ctx->get_imgui_needs_select_unfocused_light();
	if (needs_select)
	{
		if (l == needs_select)
		{
			ImGui::SetKeyboardFocusHere(-1);
		}
	}
	else if (ImGui::IsItemFocused())
	{
		m_app_ctx->set_selected_light(l);
	}

	ImGui::PopStyleVar();
	const ImVec2& cur_min = ImGui::GetItemRectMin();
	const ImVec2& cur_max = ImGui::GetItemRectMax();

	// handle controls
	ImGui::PushID(l->get_id().c_str());
	if (ImGui::BeginPopupContextItem())
	{
		m_app_ctx->set_selected_light(l);

		if (l->is_visible())
		{
			if (ImGui::MenuItem("Hide"))
			{
				l->set_visibility(false);
				m_app_ctx->scene.update_light(l);
			}
		}
		else
		{
			if (ImGui::MenuItem("Show"))
			{
				l->set_visibility(true);
				m_app_ctx->scene.update_light(l);
			}
		}

		// don't allow renaming, destroying, or duplicating camera light
		if (index != 0)
		{
			ImGui::Separator();
			if (ImGui::MenuItem("Duplicate"))
			{
				m_app_ctx->duplicate_selected_light();
				performed_destructive_action = true;
			}

			ImGui::Separator();
			if (ImGui::MenuItem("Rename"))
			{
				set_renaming_light(l);
			}
			if (ImGui::MenuItem("Destroy"))
			{
				m_app_ctx->destroy_light(l);
				performed_destructive_action = true;
			}
		}
		ImGui::EndPopup();
	}
	ImGui::PopID();

	ImGui::TreePop();
}
void scene_graph_window::handle_lights(bool& performed_destructive_action)
{
	const f32 padding_x = 3.f;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(padding_x, 2.f));
	const bool open = ImGui::TreeNodeEx("##SGW_LITS", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth, "Lights");
	ImGui::PopStyleVar();

	ImGui::PushID("##SGW_LITS");
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::MenuItem("Add Light"))
		{
			m_app_ctx->add_light();
		}
		ImGui::EndPopup();
	}
	ImGui::PopID();

	if (open)
	{
		for (u32 i = 0; i < m_app_ctx->scene.get_lights().size(); i++)
		{
			handle_light(i, performed_destructive_action);
			if (performed_destructive_action)
			{
				break;
			}
		}
		ImGui::TreePop();
	}
}
// FIXME
void scene_graph_window::handle_waypoint(waypoint* const w, bool& performed_destructive_action)
{
	const f32 padding_x = 3.f;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(padding_x, 2.f));
	if (w == m_renaming_waypoint)
	{
		const f32 x = ImGui::GetCursorPosX();

		if (!w->is_visible())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		}
		ImGui::TreeNodeEx(w->get_id().c_str(), ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth | (w == m_app_ctx->get_selected_waypoint() ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_Leaf, "%s", w->get_name().c_str());
		if (!w->is_visible())
		{
			ImGui::PopStyleColor();
		}

		constexpr u32 BUF_SIZE = 32;
		char buf[32] = { 0 };
		memcpy_s(buf, BUF_SIZE, w->get_name().c_str(), w->get_name().size());

		// FIXME kind of hacky?
		ImGui::SameLine();
		const f32 size = ImGui::GetFontSize();
		ImGui::SetCursorPosX(x + size + padding_x);

		if (ImGui::InputText("##SGW_RENAME_WAY", buf, BUF_SIZE, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			std::string new_name(buf);
			if (!new_name.empty())
			{
				w->set_name(new_name);
			}
			m_renaming_waypoint = nullptr;
		}

		if (m_rename_waypoint_needs_focus)
		{
			ImGui::SetKeyboardFocusHere(-1);
			m_rename_waypoint_needs_focus = false;
		}
		else if (!ImGui::IsItemActive())
		{
			m_renaming_waypoint = nullptr;
		}
	}
	else
	{
		if (!w->is_visible())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		}
		ImGui::TreeNodeEx(w->get_id().c_str(), ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth | (w == m_app_ctx->get_selected_waypoint() ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_Leaf, "%s", w->get_name().c_str());
		if (!w->is_visible())
		{
			ImGui::PopStyleColor();
		}
	}

	waypoint* needs_select = m_app_ctx->get_imgui_needs_select_unfocused_waypoint();
	if (needs_select)
	{
		if (w == needs_select)
		{
			ImGui::SetKeyboardFocusHere(-1);
		}
	}
	else if (ImGui::IsItemFocused())
	{
		m_app_ctx->set_selected_waypoint(w);
	}

	ImGui::PopStyleVar();
	const ImVec2& cur_min = ImGui::GetItemRectMin();
	const ImVec2& cur_max = ImGui::GetItemRectMax();

	// handle controls
	ImGui::PushID(w->get_id().c_str());
	if (ImGui::BeginPopupContextItem())
	{
		m_app_ctx->set_selected_waypoint(w);

		if (ImGui::MenuItem("Duplicate"))
		{
			m_app_ctx->duplicate_selected_waypoint();
			performed_destructive_action = true;
		}
		ImGui::Separator();

		if (ImGui::MenuItem("Rename"))
		{
			set_renaming_waypoint(w);
		}
		if (ImGui::MenuItem("Destroy"))
		{
			m_app_ctx->destroy_waypoint(w);
			performed_destructive_action = true;
		}
		ImGui::EndPopup();
	}
	ImGui::PopID();

	ImGui::TreePop();
}
void scene_graph_window::handle_waypoints(bool& performed_destructive_action)
{
	const f32 padding_x = 3.f;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(padding_x, 2.f));
	const bool open = ImGui::TreeNodeEx("##SGW_WAYS", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth, "Waypoints");
	ImGui::PopStyleVar();

	ImGui::PushID("##SGW_WAYS");
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::MenuItem("Add Waypoint"))
		{
			m_app_ctx->add_waypoint();
		}
		ImGui::EndPopup();
	}
	ImGui::PopID();

	if (open)
	{
		auto& waypoints = m_app_ctx->scene.get_waypoints();
		for (u32 i = 0; i < waypoints.size(); i++)
		{
			handle_waypoint(waypoints[i], performed_destructive_action);
			if (performed_destructive_action)
			{
				break;
			}
		}
		ImGui::TreePop();
	}
}
