#include "pch.h"
#include "scene_graph_window.h"
#include "app_ctx.h"
#include "scene_graph.h"

scene_graph_window::scene_graph_window(app_ctx* const a_ctx) :
	imgui_window(a_ctx, "Phorms")
{}

void scene_graph_window::handle_frame()
{
	sgnode* const root = m_app_ctx->scene.get_sg_root();
	handle_node(root, false);
}

scene_graph_window::Rect scene_graph_window::handle_node(sgnode* const node, const bool parent_cutted_to_clipboard)
{
	bool cutted_to_clipboard = parent_cutted_to_clipboard || (m_app_ctx->clipboard == node && m_app_ctx->clipboard_cut);
	// draw current node
	std::string display_name = node->name;
	if (!node->is_mesh())
	{
		if (display_name.empty())
		{
			display_name = operation_to_string(node->operation);
		}
		else
		{
			display_name += " [" + operation_to_string(node->operation) + "]";
		}
	}
	if (cutted_to_clipboard)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
	}

	const f32 padding_x = 3.f;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(padding_x, 2.f));
	bool open = false;
	if (node->is_renaming)
	{
		const f32 x = ImGui::GetCursorPosX();
		open = ImGui::TreeNodeEx(node->id.c_str(), ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth | (node == m_app_ctx->get_selected_sgnode() ? ImGuiTreeNodeFlags_Selected : 0) | (node->is_leaf() ? ImGuiTreeNodeFlags_Leaf : 0), "%s", display_name.c_str());

		constexpr u32 BUF_SIZE = 32;
		char buf[32] = { 0 };
		memcpy_s(buf, BUF_SIZE, node->name.c_str(), node->name.size());

		// FIXME kind of hacky?
		ImGui::SameLine();
		const f32 size = ImGui::GetFontSize();
		ImGui::SetCursorPosX(x + size + padding_x);

		if (ImGui::InputText("##SGW_RENAME", buf, BUF_SIZE, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			std::string new_name(buf);
			if (!new_name.empty())
				node->set_name(new_name);
			node->set_renaming(false);
		}
		if (m_rename_needs_focus)
		{
			ImGui::SetKeyboardFocusHere(-1);
			m_rename_needs_focus = false;
		}
		else if (!ImGui::IsItemActive())
		{
			node->set_renaming(false);
		}
	}
	else
	{
		open = ImGui::TreeNodeEx(node->id.c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth | (node == m_app_ctx->get_selected_sgnode() ? ImGuiTreeNodeFlags_Selected : 0) | (node->is_leaf() ? ImGuiTreeNodeFlags_Leaf : 0), "%s", display_name.c_str());
	}
	ImGui::PopStyleVar();
	if (cutted_to_clipboard)
	{
		ImGui::PopStyleVar();
	}
	const ImVec2& cur_min = ImGui::GetItemRectMin();
	const ImVec2& cur_max = ImGui::GetItemRectMax();

	// handle controls
	ImGui::PushID(node->id.c_str());
	if (ImGui::BeginPopupContextItem())
	{
		m_app_ctx->set_selected_sgnode(node, true);

		if (!node->is_mesh())
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
			ImGui::Separator();
		}
		if (!node->is_root() || m_app_ctx->is_node_frozen(node))
		{
			const bool is_frozen = m_app_ctx->is_node_frozen(node);
			if (!node->is_root() && !is_frozen)
				if (ImGui::MenuItem("Phreeze!"))
					m_app_ctx->freeze_action(node);
			if (is_frozen)
				if (ImGui::MenuItem("Unphreeze!"))
					m_app_ctx->unfreeze_action(node);
			ImGui::Separator();
		}
		if (ImGui::MenuItem("Rename"))
		{
			m_rename_needs_focus = true;
			node->set_renaming(true);
		}
		if (ImGui::MenuItem("Destroy"))
			m_app_ctx->destroy_selected_action();

		ImGui::EndPopup();
	}
	ImGui::PopID();

	if (ImGui::IsItemFocused())
	{
		m_app_ctx->set_selected_sgnode(node, true);
	}

	if (open)
	{
		// draw children if it has any
		if (!node->is_leaf())
		{
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			const ImColor line_color(128, 128, 128);
			// start vertical line at position of current node
			ImVec2 vertical_start = ImGui::GetCursorScreenPos();
			vertical_start.x -= 8.f;
			vertical_start.y -= 8.f;
			ImVec2 vertical_end = vertical_start;
			for (sgnode* child : node->children)
			{
				const Rect& child_rect = handle_node(child, cutted_to_clipboard);
				// draw horizontal line from vertical line to current child
				const f32 horizontal_size = child->is_leaf() ? 24.f : 12.f;
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
