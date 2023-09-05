#include "pch.h"
#include "scene_graph_window.h"
#include "app_ctx.h"
#include "core/sgnode.h"
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
			m_app_ctx->set_sel_type(global_selection_type::sgnode);
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
	sgnode* const root = m_app_ctx->scene.get_sg_root();
	handle_node(root);
	m_app_ctx->unset_imgui_needs_select_unfocused_sgnode();
	handle_lights();
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
scene_graph_window::rect scene_graph_window::handle_node(sgnode* const node)
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
		open = ImGui::TreeNodeEx(node->get_id().c_str(), ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth | (node == m_app_ctx->get_selected_sgnode() ? ImGuiTreeNodeFlags_Selected : 0) | (node->get_children().size() == 0 ? ImGuiTreeNodeFlags_Leaf : 0), "%s", display_name.c_str());

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
		open = ImGui::TreeNodeEx(node->get_id().c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth | (node == m_app_ctx->get_selected_sgnode() ? ImGuiTreeNodeFlags_Selected : 0) | (node->get_children().size() == 0 ? ImGuiTreeNodeFlags_Leaf : 0), "%s", display_name.c_str());
	}

	sgnode* needs_select = m_app_ctx->get_imgui_needs_select_unfocused_sgnode();
	if (needs_select)
	{
		if (node == needs_select)
		{
			ImGui::SetKeyboardFocusHere(-1);
		}
	}
	else if (ImGui::IsItemFocused())
	{
		m_app_ctx->set_selected_sgnode(node);
	}

	ImGui::PopStyleVar();
	const ImVec2& cur_min = ImGui::GetItemRectMin();
	const ImVec2& cur_max = ImGui::GetItemRectMax();

	// handle controls
	ImGui::PushID(node->get_id().c_str());
	if (ImGui::BeginPopupContextItem())
	{
		m_app_ctx->set_selected_sgnode(node);

		if (node->is_operation())
		{
			if (ImGui::MenuItem("Add Cube"))
				m_app_ctx->create_cube_action();
			if (ImGui::MenuItem("Add Sphere"))
				m_app_ctx->create_sphere_action();
			if (ImGui::MenuItem("Add Cylinder"))
				m_app_ctx->create_cylinder_action();
			if (ImGui::MenuItem("Add Cone"))
				m_app_ctx->create_cone_action();
			if (ImGui::MenuItem("Add Torus"))
				m_app_ctx->create_torus_action();
			if (ImGui::MenuItem("Add Heightmap"))
				m_app_ctx->create_heightmap_action();
			ImGui::Separator();
			if (ImGui::MenuItem("Add Union"))
				m_app_ctx->create_union_action();
			if (ImGui::MenuItem("Add Subtract"))
				m_app_ctx->create_subtract_action();
			if (ImGui::MenuItem("Add Intersect"))
				m_app_ctx->create_intersect_action();
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
					m_app_ctx->unfreeze_action(node);
				ImGui::Separator();
			}
			// if the current node is NOT a clone of an original frozen node
			else if (!node->is_frozen())
			{
				if (ImGui::MenuItem("Phreeze!"))
					m_app_ctx->freeze_action(node);
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
			m_app_ctx->destroy_selected_action();

		ImGui::EndPopup();
	}
	ImGui::PopID();

	if (open)
	{
		// draw children if it has any
		if (node->get_children().size())
		{
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			const ImColor line_color(128, 128, 128);
			// start vertical line at position of current node
			ImVec2 vertical_start = ImGui::GetCursorScreenPos();
			vertical_start.x -= 8.f;
			vertical_start.y -= 8.f;
			ImVec2 vertical_end = vertical_start;
			for (sgnode* child : node->get_children())
			{
				const rect& child_rect = handle_node(child);
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
void scene_graph_window::handle_lights()
{
	const f32 padding_x = 3.f;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(padding_x, 2.f));
	const bool open = ImGui::TreeNodeEx("##SGW_LIGHTS", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth, "Lights");
	ImGui::PopStyleVar();

	ImGui::PushID("##SW_LIGHTS");
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
		auto& lights = m_app_ctx->scene.get_lights();
		for (u32 i = 0; i < lights.size(); i++)
		{
			light* light = lights.data() + i;
			if (ImGui::MenuItem(light->name.c_str(), "", m_app_ctx->get_selected_light() == light))
			{
				m_app_ctx->set_selected_light(light);
			}
		}
		ImGui::TreePop();
	}
}
