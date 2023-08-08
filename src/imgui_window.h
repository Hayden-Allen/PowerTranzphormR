#pragma once

class imgui_window {
public:
	imgui_window() {}
	virtual ~imgui_window() {}

	virtual void handle_frame() = 0;

	std::string title;
};
