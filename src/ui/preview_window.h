#pragma once
#include "imgui_window.h"

class preview_window : public imgui_window
{
public:
	preview_window(app_ctx* const a_ctx);
	virtual ~preview_window() {}
public:
	void handle_frame() override;
	void set_enable_callback(const std::function<void()>& callback);
private:
	std::function<void()> m_enable_callback;
	bool m_was_using_imguizmo = false;
	tmat<space::OBJECT, space::PARENT> m_imguizmo_undo_mat;
};
