#include "pch.h"
#include "scene_graph_window.h"

scene_graph_window::scene_graph_window(app_ctx* const a_ctx) :
	imgui_window(a_ctx, "Scene Graph")
{}

void scene_graph_window::handle_frame()
{
	sgnode* const root = m_app_ctx->scene.get_sg_root();
	handle_node(root);
}

scene_graph_window::Rect scene_graph_window::handle_node(sgnode* const node) const
{
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
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.f, 2.f));
	const bool open = ImGui::TreeNodeEx(node->id.c_str(), ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen | (node == m_app_ctx->scene.get_selected_node() ? ImGuiTreeNodeFlags_Selected : 0) | (node->is_leaf() ? ImGuiTreeNodeFlags_Leaf : 0), "%s", display_name.c_str());
	ImGui::PopStyleVar();
	const ImVec2& cur_min = ImGui::GetItemRectMin();
	const ImVec2& cur_max = ImGui::GetItemRectMax();

	// handle controls
	ImGui::PushID(node->id.c_str());
	if (ImGui::BeginPopupContextItem())
	{
		m_app_ctx->scene.set_selected_node(node);
		if (ImGui::MenuItem("Add Union"))
		{
			//
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Add Cube"))
		{
			m_app_ctx->create_cube_action();
		}
		if (ImGui::MenuItem("Add Sphere"))
		{
			m_app_ctx->create_sphere_action();
		}
		if (ImGui::MenuItem("Add Cylinder"))
		{
			m_app_ctx->create_cylinder_action();
		}
		if (ImGui::MenuItem("Add Cone"))
		{
			m_app_ctx->create_cone_action();
		}
		if (ImGui::MenuItem("Add Torus"))
		{
			m_app_ctx->create_torus_action();
		}
		if (ImGui::MenuItem("Add Heightmap"))
		{
			m_app_ctx->create_heightmap_action();
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Destroy"))
		{
			m_app_ctx->destroy_selected_action();
		}
		if (!node->is_mesh())
		{
			ImGui::Separator();
			if (ImGui::MenuItem("Add Union"))
			{
				m_app_ctx->create_union_action();
			}
			if (ImGui::MenuItem("Add Subtract"))
			{
				m_app_ctx->create_subtract_action();
			}
			if (ImGui::MenuItem("Add Intersect"))
			{
				m_app_ctx->create_intersect_action();
			}
		}

		ImGui::EndPopup();
	}
	ImGui::PopID();

	if (ImGui::IsItemClicked())
	{
		//
		// TODO
		//
		m_app_ctx->scene.set_selected_node(node);
	}
	if (ImGui::BeginDragDropTarget())
	{
		//
		// TODO
		//
		ImGui::EndDragDropTarget();
	}
	if (ImGui::BeginDragDropSource())
	{
		//
		// TODO
		//
		ImGui::EndDragDropSource();
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
				const Rect& child_rect = handle_node(child);
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

std::string scene_graph_window::operation_to_string(carve::csg::CSG::OP op)
{
	switch (op)
	{
	case carve::csg::CSG::OP::ALL:
		return "All";
	case carve::csg::CSG::OP::A_MINUS_B:
		return "Subtract (A - B)";
	case carve::csg::CSG::OP::B_MINUS_A:
		return "Subtract (B - A)";
	case carve::csg::CSG::OP::INTERSECTION:
		return "Intersect";
	case carve::csg::CSG::OP::SYMMETRIC_DIFFERENCE:
		return "Difference (Symmetric)";
	case carve::csg::CSG::OP::UNION:
		return "Union";
	default:
		return "<ERROR>";
	}
}
