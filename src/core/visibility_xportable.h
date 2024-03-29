#pragma once
#include "pch.h"
#include "xportable.h"

class visibility_xportable : public xportable
{
public:
	visibility_xportable();
	visibility_xportable(const std::string& name);
	visibility_xportable(const nlohmann::json& obj);
public:
	virtual void set_visibility(const bool v);
	bool is_visible() const;
	virtual nlohmann::json save() const override;
	void copy_properties_from(const visibility_xportable& src);
protected:
	bool m_visible = true;
};