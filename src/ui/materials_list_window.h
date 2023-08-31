#pragma once
#include "imgui_window.h"

class materials_list_window : public imgui_window
{
public:
	materials_list_window(app_ctx* const a_ctx);
	virtual ~materials_list_window() {}
public:
	virtual void handle_focused(bool focused) override;
	virtual void handle_frame() override;
private:
	bool m_was_focused = false;
};
