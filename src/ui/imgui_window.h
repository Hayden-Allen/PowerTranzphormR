#pragma once

struct app_ctx;

class imgui_window
{
public:
	imgui_window(app_ctx* const a_ctx, const std::string& title) :
		m_app_ctx(a_ctx),
		m_title(title)
	{}
	virtual ~imgui_window() {}
public:
	virtual void handle_focused(bool focused) {}
	virtual void handle_frame() = 0;
	const std::string& get_title() const
	{
		return m_title;
	}
protected:
	app_ctx* const m_app_ctx = nullptr;
private:
	std::string m_title;
};
