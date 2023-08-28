#pragma once
#include "pch.h"
#include "carve.h"
#include "scene_ctx.h"

constexpr static f32 MIN_PARAM_VALUE = .01f, MAX_PARAM_VALUE = 4096.f, DRAG_PARAM_STEP = .01f;
struct generated_mesh_param
{
	bool is_float;
	void* value;
	f32 min, max, speed;
};

class generated_mesh
{
public:
	mesh_t* mesh;
	std::vector<point<space::OBJECT>> src_verts;
	bool dirty;
public:
	generated_mesh(mesh_t* const m);
	MGL_DCM(generated_mesh);
	virtual ~generated_mesh();
public:
	static generated_mesh* create(const nlohmann::json& obj);
public:
	virtual std::unordered_map<std::string, generated_mesh_param> get_params() const;
	virtual void recompute(scene_ctx* const scene);
	virtual generated_mesh* clone() const;
	virtual generated_mesh* clone(const tmat<space::OBJECT, space::WORLD>& old_transform) const;
	virtual nlohmann::json save() const;
	virtual GLuint get_material() const;
	virtual void set_material(const GLuint mat);
protected:
	virtual primitive_options* get_options() const;
	void copy_verts();
};

class generated_primitive : public generated_mesh
{
public:
	generated_primitive(mesh_t* const m, const GLuint material);
	generated_primitive(const nlohmann::json& obj);
	MGL_DCM(generated_primitive);
	virtual ~generated_primitive();
public:
	std::unordered_map<std::string, generated_mesh_param> get_params() const override;
	virtual void recompute(scene_ctx* const scene) override;
	GLuint get_material() const override;
	void set_material(const GLuint mat) override;
	nlohmann::json save() const override;
protected:
	GLuint m_material;
};

class generated_cuboid : public generated_primitive
{
public:
	generated_cuboid(scene_ctx* const scene, const GLuint material, const cuboid_options& opts);
	generated_cuboid(const nlohmann::json& obj);
public:
	std::unordered_map<std::string, generated_mesh_param> get_params() const override;
	void recompute(scene_ctx* const scene) override;
	virtual generated_mesh* clone() const;
	nlohmann::json save() const override;
protected:
	primitive_options* get_options() const override;
private:
	generated_cuboid(const GLuint material, const cuboid_options& opts);
private:
	cuboid_options m_options;
};

class generated_ellipsoid : public generated_primitive
{
public:
	generated_ellipsoid(scene_ctx* const scene, const GLuint material, const ellipsoid_options& opts);
	generated_ellipsoid(const nlohmann::json& obj);
public:
	std::unordered_map<std::string, generated_mesh_param> get_params() const override;
	void recompute(scene_ctx* const scene) override;
	virtual generated_mesh* clone() const;
	nlohmann::json save() const override;
protected:
	primitive_options* get_options() const override;
private:
	generated_ellipsoid(const GLuint material, const ellipsoid_options& opts);
private:
	ellipsoid_options m_options;
};

class generated_cylinder : public generated_primitive
{
public:
	generated_cylinder(scene_ctx* const scene, const GLuint material, const cylinder_options& opts);
	generated_cylinder(const nlohmann::json& obj);
public:
	std::unordered_map<std::string, generated_mesh_param> get_params() const override;
	void recompute(scene_ctx* const scene) override;
	virtual generated_mesh* clone() const;
	nlohmann::json save() const override;
protected:
	primitive_options* get_options() const override;
private:
	generated_cylinder(const GLuint material, const cylinder_options& opts);
private:
	cylinder_options m_options;
};

class generated_cone : public generated_primitive
{
public:
	generated_cone(scene_ctx* const scene, const GLuint material, const cone_options& opts);
	generated_cone(const nlohmann::json& obj);
public:
	std::unordered_map<std::string, generated_mesh_param> get_params() const override;
	void recompute(scene_ctx* const scene) override;
	virtual generated_mesh* clone() const;
	nlohmann::json save() const override;
protected:
	primitive_options* get_options() const override;
private:
	generated_cone(const GLuint material, const cone_options& opts);
private:
	cone_options m_options;
};

class generated_torus : public generated_primitive
{
public:
	generated_torus(scene_ctx* const scene, const GLuint material, const torus_options& opts);
	generated_torus(const nlohmann::json& obj);
public:
	std::unordered_map<std::string, generated_mesh_param> get_params() const override;
	void recompute(scene_ctx* const scene) override;
	virtual generated_mesh* clone() const;
	nlohmann::json save() const override;
protected:
	primitive_options* get_options() const override;
private:
	generated_torus(const GLuint material, const torus_options& opts);
private:
	torus_options m_options;
};

class generated_heightmap : public generated_primitive
{
public:
	generated_heightmap(scene_ctx* const scene, const GLuint material, const heightmap_options& opts);
	generated_heightmap(const nlohmann::json& obj);
public:
	std::unordered_map<std::string, generated_mesh_param> get_params() const override;
	void recompute(scene_ctx* const scene) override;
	virtual generated_mesh* clone() const;
	nlohmann::json save() const override;
protected:
	primitive_options* get_options() const override;
private:
	generated_heightmap(const GLuint material, const heightmap_options& opts);
private:
	heightmap_options m_options;
};
