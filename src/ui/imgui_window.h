#pragma once

class imgui_window
{
public:
	imgui_window(const std::string& title) :
		m_title(title)
	{}
	virtual ~imgui_window() {}
public:
	virtual void handle_frame() = 0;
	const std::string& get_title() const
	{
		return m_title;
	}
private:
	std::string m_title;
};
