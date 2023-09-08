#pragma once
#include "imgui_window.h"
#include "geom/carve.h"

enum class vertex_editor_mode
{
	POSITION,
	COLOR,
	UV,
	COUNT
};
static std::string vertex_editor_mode_string(const vertex_editor_mode em)
{
	switch (em)
	{
	case vertex_editor_mode::POSITION: return "Position";
	case vertex_editor_mode::COLOR: return "Color";
	case vertex_editor_mode::UV: return "UVs";
	}
	assert(false);
	return "";
}

class vertex_editor_window : public imgui_window
{
public:
	vertex_editor_window(app_ctx* const app_ctx);
	virtual ~vertex_editor_window() {}
public:
	void handle_focused(const bool focused) override;
	void handle_frame() override;
private:
	vertex_editor_mode m_mode = vertex_editor_mode::POSITION;
private:
	bool handle_frame_base(const mesh_t* const mesh, std::vector<point<space::OBJECT>>& local_verts, const tmat<space::OBJECT, space::WORLD>& mat);
};