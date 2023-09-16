#pragma once
#include "pch.h"
#include "carve.h"
#include "util/color.h"

class scene_ctx;

enum class generated_mesh_param_type
{
	NONE = -1,
	FLOAT_1,
	FLOAT_1_LOG,
	FLOAT_2,
	FLOAT_4,
	FLOAT_4_SUM_1,
	COLOR_4,
	UINT_1,
	UINT_1_LOG,
	UINT_2,
	UINT_2_LOG,
	UINT_2_DIRECT
};
constexpr static f32 MIN_PARAM_VALUE = .01f, MAX_PARAM_VALUE = 4096.f, DRAG_PARAM_STEP = .01f;
struct generated_mesh_param
{
	generated_mesh_param_type type;
	void* value;
	f32 min, max, speed;
};

class generated_mesh
{
public:
	mesh_t* mesh;
	bool dirty;
public:
	generated_mesh(mesh_t* const m);
	MGL_DCM(generated_mesh);
	virtual ~generated_mesh();
public:
	static generated_mesh* create(const nlohmann::json& obj, scene_ctx* const scene);
public:
	virtual std::vector<std::pair<std::string, generated_mesh_param>> get_params() const;
	virtual void recompute(scene_ctx* const scene);
	// for operation nodes
	virtual generated_mesh* clone(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& inv_mat = tmat<space::WORLD, space::OBJECT>()) const;
	virtual nlohmann::json save(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& mat) const;
	virtual GLuint get_material() const;
	virtual void set_material(scene_ctx* const scene, const GLuint mat);
	virtual void replace_material(scene_ctx* const scene, const GLuint old_mat, const GLuint new_mat);
	virtual void set_mesh(mesh_t* const m);
	virtual void set_vertices_color(const color_t& col, scene_ctx* const scene);
	virtual void center_verts_at_origin(const tmat<space::OBJECT, space::PARENT>& mat = tmat<space::OBJECT, space::PARENT>());
	void set_dirty();
	bool is_dirty() const;
	void clear();
	void copy_mesh_from(const generated_mesh* const other, scene_ctx* const scene);
	virtual mesh_t* clone_mesh_to_local(scene_ctx* const scene, const tmat<space::OBJECT, space::WORLD>& mat) const;
	virtual bool is_static() const;
	virtual tex_coord_t get_uv_offset() const;
protected:
	virtual primitive_options* get_options() const;
};

class generated_primitive : public generated_mesh
{
public:
	MGL_DCM(generated_primitive);
	virtual ~generated_primitive();
public:
	std::vector<std::pair<std::string, generated_mesh_param>> get_params() const override;
	virtual void recompute(scene_ctx* const scene) override;
	GLuint get_material() const override;
	void set_material(scene_ctx* const scene, const GLuint mat) override;
	void replace_material(scene_ctx* const scene, const GLuint old_mat, const GLuint new_mat) override;
	nlohmann::json save(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& mat) const override;
protected:
	GLuint m_material;
protected:
	generated_primitive(mesh_t* const m, const GLuint material);
	generated_primitive(const nlohmann::json& obj);
protected:
	void set_options(const nlohmann::json& obj);
};

class generated_cuboid : public generated_primitive
{
public:
	generated_cuboid(scene_ctx* const scene, const GLuint material, const cuboid_options& opts);
	generated_cuboid(const nlohmann::json& obj);
	MGL_DCM(generated_cuboid);
public:
	std::vector<std::pair<std::string, generated_mesh_param>> get_params() const override;
	void recompute(scene_ctx* const scene) override;
	generated_mesh* clone(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& inv_mat = tmat<space::WORLD, space::OBJECT>()) const override;
	nlohmann::json save(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& mat) const override;
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
	MGL_DCM(generated_ellipsoid);
public:
	std::vector<std::pair<std::string, generated_mesh_param>> get_params() const override;
	void recompute(scene_ctx* const scene) override;
	generated_mesh* clone(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& inv_mat = tmat<space::WORLD, space::OBJECT>()) const override;
	nlohmann::json save(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& mat) const override;
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
	MGL_DCM(generated_cylinder);
public:
	std::vector<std::pair<std::string, generated_mesh_param>> get_params() const override;
	void recompute(scene_ctx* const scene) override;
	generated_mesh* clone(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& inv_mat = tmat<space::WORLD, space::OBJECT>()) const override;
	nlohmann::json save(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& mat) const override;
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
	MGL_DCM(generated_cone);
public:
	std::vector<std::pair<std::string, generated_mesh_param>> get_params() const override;
	void recompute(scene_ctx* const scene) override;
	generated_mesh* clone(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& inv_mat = tmat<space::WORLD, space::OBJECT>()) const override;
	nlohmann::json save(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& mat) const override;
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
	MGL_DCM(generated_torus);
public:
	std::vector<std::pair<std::string, generated_mesh_param>> get_params() const override;
	void recompute(scene_ctx* const scene) override;
	generated_mesh* clone(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& inv_mat = tmat<space::WORLD, space::OBJECT>()) const override;
	nlohmann::json save(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& mat) const override;
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
	MGL_DCM(generated_heightmap);
public:
	std::vector<std::pair<std::string, generated_mesh_param>> get_params() const override;
	void recompute(scene_ctx* const scene) override;
	generated_mesh* clone(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& inv_mat = tmat<space::WORLD, space::OBJECT>()) const override;
	nlohmann::json save(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& mat) const override;
protected:
	primitive_options* get_options() const override;
private:
	generated_heightmap(const GLuint material, const heightmap_options& opts);
private:
	heightmap_options m_options;
};

class generated_static_mesh : public generated_mesh
{
public:
	generated_static_mesh(mesh_t* const m, scene_ctx* const scene);
	generated_static_mesh(const nlohmann::json& obj, scene_ctx* const scene);
	MGL_DCM(generated_static_mesh);
public:
	void set_material(scene_ctx* const scene, const GLuint new_mat) override;
	void replace_material(scene_ctx* const scene, const GLuint old_mat, const GLuint new_mat) override;
	generated_mesh* clone(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& inv_mat = tmat<space::WORLD, space::OBJECT>()) const override;
	nlohmann::json save(scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& mat) const override;
	bool is_static() const override;
	void set_mesh(mesh_t* const m) override;
	GLuint get_material() const override;
	void set_vertices_color(const color_t& col, scene_ctx* const scene) override;
	void center_verts_at_origin(const tmat<space::OBJECT, space::PARENT>& inv_mat = tmat<space::OBJECT, space::PARENT>()) override;
	std::vector<std::pair<std::string, generated_mesh_param>> get_params() const override;
	tex_coord_t get_uv_offset() const override;
private:
	void check_material_id(scene_ctx* const scene);
private:
	GLuint m_material = MAX_VALUE_TYPE(GLuint);
	tex_coord_t m_uv_offset;
};
