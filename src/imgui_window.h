#pragma once

class imgui_window
{
public:
	std::string title;
public:
	imgui_window() {}
	virtual ~imgui_window() {}
public:
	virtual void handle_frame() = 0;
};
