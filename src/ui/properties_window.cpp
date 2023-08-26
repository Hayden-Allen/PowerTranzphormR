#include "pch.h"
#include "properties_window.h"

properties_window::properties_window(app_ctx* const a_ctx) :
	imgui_window(a_ctx, "Properties")
{
}

void properties_window::handle_frame()
{
	sgnode* const selected = m_app_ctx->scene.get_selected_node();
	assert(selected);

	handle_transform(selected);
	if (selected->is_mesh())
	{
		handle_mesh(selected);
	}
}

void properties_window::handle_transform(sgnode* const selected)
{
	//
}

void properties_window::handle_mesh(sgnode* const selected)
{
	for (const auto& prop : selected->gen->get_params())
	{
		if (prop.second.is_drag)
		{
			if (prop.second.is_float)
			{
				if (ImGui::DragFloat(prop.first.c_str(), static_cast<f32*>(prop.second.value), prop.second.speed, prop.second.min, prop.second.max))
				{
					selected->set_gen_dirty();
				}
			}
			else
			{
				if (ImGui::DragInt(prop.first.c_str(), static_cast<s32*>(prop.second.value), prop.second.speed, static_cast<s32>(prop.second.min), static_cast<s32>(prop.second.max)))
				{
					selected->set_gen_dirty();
				}
			}
		}
		else
		{
			if (prop.second.is_float)
			{
				if (ImGui::SliderFloat(prop.first.c_str(), static_cast<f32*>(prop.second.value), prop.second.min, prop.second.max))
				{
					selected->set_gen_dirty();
				}
			}
			else
			{
				if (ImGui::SliderInt(prop.first.c_str(), static_cast<s32*>(prop.second.value), static_cast<s32>(prop.second.min), static_cast<s32>(prop.second.max)))
				{
					selected->set_gen_dirty();
				}
			}
		}
	}
}
