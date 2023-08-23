#include "pch.h"
#include "scene_graph_window.h"

scene_graph_window::scene_graph_window(scene_ctx* scene)
{
	title = "Scene Graph";
	m_scene = scene;
}

void scene_graph_window::handle_frame()
{
	sgnode* root = m_scene->get_sg_root();
	handle_node(root);
}

void scene_graph_window::handle_node(sgnode* node)
{
	std::string display_name = node->name;
	if (!node->is_leaf())
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
	bool open = ImGui::TreeNodeEx(node->id.c_str(), ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen | (node->selected ? ImGuiTreeNodeFlags_Selected : 0) | (node->is_leaf() ? ImGuiTreeNodeFlags_Leaf : 0), "%s", display_name.c_str());

	ImGui::PushID(node->id.c_str());
	if (ImGui::BeginPopupContextItem())
	{
		//
		// TODO
		//
		ImGui::EndPopup();
	}
	ImGui::PopID();

	if (ImGui::IsItemClicked())
	{
		//
		// TODO
		//
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
		for (sgnode* child : node->children) {
			handle_node(child);
		}
		ImGui::TreePop();
	}
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
		return "";
	}
}
