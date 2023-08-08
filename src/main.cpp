#include "pch.h"
#include "../lib/glm/glm/glm.hpp"
#include "../lib/glm/glm/ext/matrix_transform.hpp"
#include "../lib/glm/glm/ext/matrix_clip_space.hpp"

using namespace hats;

#if defined(GLU_TESS_CALLBACK_VARARGS)
typedef GLvoid(__stdcall* GLUTessCallback)(...);
#else
typedef void(__stdcall* GLUTessCallback)();
#endif

struct camera_t {
	camera_t() :
		pos(0, 0, 0)
	{}
	~camera_t() {}

	void handle_key(int key, bool down) {
		switch (key) {
		case GLFW_KEY_W:
			key_w = down;
			break;

		case GLFW_KEY_S:
			key_s = down;
			break;

		case GLFW_KEY_A:
			key_a = down;
			break;

		case GLFW_KEY_D:
			key_d = down;
			break;

		case GLFW_KEY_LEFT_SHIFT:
		case GLFW_KEY_RIGHT_SHIFT:
			key_shift = down;
			break;

		default:
			break;
		}
	}

	void unset_all_keys() {
		key_w = false;
		key_s = false;
		key_a = false;
		key_d = false;
		key_shift = false;
	}

	void handle_cursor(double dx, double dy) {
		static constexpr float pi = (float)M_PI;

		angle_y += static_cast<float>(dx) * 0.005f;
		while (angle_y < 0.0f) {
			angle_y += pi * 2.0f;
		}
		while (angle_y >= pi * 2.0f) {
			angle_y -= pi * 2.0f;
		}

		angle_x += static_cast<float>(dy) * 0.005f;
		if (angle_x < -pi * 0.5f) {
			angle_x = -pi * 0.5f;
		}
		if (angle_x > pi * 0.5f) {
			angle_x = pi * 0.5f;
		}

	}

	void update(double dt) {
		glm::vec3 dir = glm::vec3(0.0f, 0.0f, 0.0f);
		if (key_w) {
			dir.z -= 1.0f;
		}
		if (key_s) {
			dir.z += 1.0f;
		}
		if (key_a) {
			dir.x -= 1.0f;
		}
		if (key_d) {
			dir.x += 1.0f;
		}
		dir *= (f32)dt * (key_shift ? 1.0f : 5.0f);

		view_mtx = glm::mat4(1.0f);
		view_mtx = glm::rotate(view_mtx, angle_x, glm::vec3(1.0f, 0.0f, 0.0f));
		view_mtx = glm::rotate(view_mtx, angle_y, glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 inv_view_mtx = glm::inverse(view_mtx);
		dir = inv_view_mtx * glm::vec4(dir, 1.0f);
		pos += dir;

		view_mtx = glm::translate(view_mtx, -pos);
	}

	glm::mat4 view_mtx = glm::mat4(1.0f);

	bool key_w = false, key_s = false, key_a = false, key_d = false, key_shift = false;
	float angle_x = 0.0f, angle_y = 0.0f;
	glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f);
};

namespace globals {
	camera_t camera;
	double vp_cur_prev_x = -1, vp_cur_prev_y = -1;
	double vp_prev_time = 0.0;
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
	globals::camera.handle_cursor(xpos - globals::vp_cur_prev_x, ypos - globals::vp_cur_prev_y);
	globals::vp_cur_prev_x = xpos;
	globals::vp_cur_prev_y = ypos;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		if (glfwRawMouseMotionSupported()) {
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
		}
		glfwSetCursorPosCallback(window, nullptr);
		glfwSetKeyCallback(window, nullptr);
		globals::camera.unset_all_keys();
		return;
	}

	if (action == GLFW_PRESS) {
		globals::camera.handle_key(key, true);
	}
	else if (action == GLFW_RELEASE) {
		globals::camera.handle_key(key, false);
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == 0 && action == GLFW_PRESS) {
		glfwSetKeyCallback(window, key_callback);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		if (glfwRawMouseMotionSupported()) {
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}
		glfwGetCursorPos(window, &globals::vp_cur_prev_x, &globals::vp_cur_prev_y);
		glfwSetCursorPosCallback(window, cursor_pos_callback);
	}
}

struct tess_vtx {
	double x = 0.0, y = 0.0, z = 0.0;
	float u = 0.0f, v = 0.0f;
	std::vector<GLfloat>* target = nullptr;
};

void tess_callback_begin(GLenum type) {
	if (type != GL_TRIANGLES) {
		std::cerr << "Error: tess_callback_begin received value: " << type << "\n";
	}
}

void tess_callback_vertex_data(void* vertex_data, void* user_data) {
	tess_vtx* v = (tess_vtx*)vertex_data;
	v->target->push_back((GLfloat)v->x);
	v->target->push_back((GLfloat)v->y);
	v->target->push_back((GLfloat)v->z);
	v->target->push_back((GLfloat)v->u);
	v->target->push_back((GLfloat)v->v);
}

void tess_callback_end(void) {
	//
}

void tess_callback_edge_flag(GLboolean flag) {
	//
}

const char* VTX_SHADER_SRC = R"(
#version 430 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_tex;
uniform mat4 u_mvp;
out vec2 v_tex;
void main() {
	v_tex = a_tex;
	gl_Position = u_mvp * vec4(a_pos, 1.0);
}
)";

const char* FRAG_SHADER_SRC = R"(
#version 430 core
uniform vec3 u_col;
uniform sampler2D u_tex;
in vec2 v_tex;
out vec4 o_col;
void main() {
	o_col = vec4(u_col * texture(u_tex, v_tex).xyz, 1.0);
}
)";

struct tex_coord_t {
	float u;
	float v;
	tex_coord_t() : u(0.0f), v(0.0f) {}
	tex_coord_t(float s, float t) : u(s), v(t) {}
};
tex_coord_t operator*(double s, const tex_coord_t& t) {
	return tex_coord_t(t.u * (float)s, t.v * (float)s);
}
tex_coord_t& operator+=(tex_coord_t& t1, const tex_coord_t& t2) {
	t1.u += t2.u;
	t1.v += t2.v;
	return t1;
}

struct material_t {
	GLfloat r = 1.0f, g = 1.0f, b = 1.0f;
};

typedef carve::mesh::MeshSet<3> mesh_3_t;
typedef carve::interpolate::FaceVertexAttr<tex_coord_t> tex_coord_attr_t;
typedef carve::interpolate::FaceAttr<GLuint> mtl_id_attr_t;

template<space SPACE>
carve::geom3d::Vector hats2carve(const point<SPACE>& p)
{
	return carve::geom::VECTOR(p.x, p.y, p.z);
}

struct cylinder_options
{
	float top_radius = 1.f, bottom_radius = 1.f, height = 2.f;
	uint32_t num_steps = 16;
	tmat<space::OBJECT, space::OBJECT> transform = tmat<space::OBJECT, space::OBJECT>();
};
mesh_3_t* textured_cylinder(tex_coord_attr_t& tex_coord_attr, mtl_id_attr_t& mtl_id_attr, GLuint mtl_id, const cylinder_options& options = {})
{
	assert(options.num_steps > 0);
	assert(options.top_radius > 0.f);
	assert(options.bottom_radius > 0.f);
	assert(options.height > 0.f);

	const uint32_t STEPS = options.num_steps;
	const float DTHETA = (float)M_TWOPI / STEPS;
	const float radii[2] = { options.bottom_radius, options.top_radius };
	std::vector<mesh_3_t::vertex_t*> vertices;
	vertices.reserve(2 * STEPS);
	for (int i = 0; i < 2; i++)
	{
		const float r = radii[i];
		for (uint32_t j = 0; j < STEPS; j++)
		{
			const float x = r * cosf(j * DTHETA), z = r * sinf(j * DTHETA);
			const float y = (i * 2.f - 1.f) * options.height / 2;
			// vertices.push_back(new mesh_3_t::vertex_t(options.transform * carve::geom::VECTOR(x, y, z)));
			vertices.push_back(new mesh_3_t::vertex_t(hats2carve(point<space::OBJECT>(x, y, z).transform_copy(options.transform))));
		}
	}

	std::vector<mesh_3_t::face_t*> faces;
	faces.reserve(STEPS + 2);
	// bottom face is wound clockwise from below
	faces.push_back(new mesh_3_t::face_t(vertices.begin(), vertices.begin() + STEPS));
	// top face is wound clockwise from above
	faces.push_back(new mesh_3_t::face_t(vertices.rbegin(), vertices.rbegin() + STEPS));
	for (const auto face : faces)
	{
		for (uint32_t i = 0; i < STEPS; i++)
		{
			const float u = (cosf(i * DTHETA) + 1) / 2, v = (sinf(i * DTHETA) + 1) / 2;
			tex_coord_attr.setAttribute(face, i, tex_coord_t(u, v));
		}
	}
	for (uint32_t i = 0; i < STEPS; i++)
	{
		const int a = i, b = (i + 1) % STEPS;
		const int c = i + STEPS, d = b + STEPS;
		mesh_3_t::face_t* face = new mesh_3_t::face_t(vertices[a], vertices[c], vertices[d], vertices[b]);
		tex_coord_attr.setAttribute(face, 0, tex_coord_t(1.f * i / STEPS, 0));
		tex_coord_attr.setAttribute(face, 1, tex_coord_t(1.f * i / STEPS, 1));
		tex_coord_attr.setAttribute(face, 2, tex_coord_t(1.f * (i + 1) / STEPS, 1));
		tex_coord_attr.setAttribute(face, 3, tex_coord_t(1.f * (i + 1) / STEPS, 0));
		faces.push_back(face);
	}

	for (const auto& face : faces)
	{
		mtl_id_attr.setAttribute(face, mtl_id);
	}

	return new mesh_3_t(faces);
}

struct cuboid_options
{
	float width = 2.f, height = 2.f, depth = 2.f;
	tmat<space::OBJECT, space::OBJECT> transform = tmat<space::OBJECT, space::OBJECT>();
};
mesh_3_t* textured_cuboid(tex_coord_attr_t& tex_coord_attr, mtl_id_attr_t& mtl_id_attr, GLuint mtl_id, const cuboid_options& options = {})
{
	assert(options.width > 0.f);
	assert(options.height > 0.f);
	assert(options.depth > 0.f);

	const float w = options.width / 2, h = options.height / 2, d = options.depth / 2;
	const auto& mtx = options.transform;
	std::vector<mesh_3_t::vertex_t> v;
	v.reserve(8);
	v.push_back(mesh_3_t::vertex_t(hats2carve(point<space::OBJECT>(+w, +h, +d).transform_copy(options.transform))));
	v.push_back(mesh_3_t::vertex_t(hats2carve(point<space::OBJECT>(-w, +h, +d).transform_copy(options.transform))));
	v.push_back(mesh_3_t::vertex_t(hats2carve(point<space::OBJECT>(-w, -h, +d).transform_copy(options.transform))));
	v.push_back(mesh_3_t::vertex_t(hats2carve(point<space::OBJECT>(+w, -h, +d).transform_copy(options.transform))));
	v.push_back(mesh_3_t::vertex_t(hats2carve(point<space::OBJECT>(+w, +h, -d).transform_copy(options.transform))));
	v.push_back(mesh_3_t::vertex_t(hats2carve(point<space::OBJECT>(-w, +h, -d).transform_copy(options.transform))));
	v.push_back(mesh_3_t::vertex_t(hats2carve(point<space::OBJECT>(-w, -h, -d).transform_copy(options.transform))));
	v.push_back(mesh_3_t::vertex_t(hats2carve(point<space::OBJECT>(+w, -h, -d).transform_copy(options.transform))));

	std::vector<mesh_3_t::face_t*> faces;
	faces.reserve(6);
	faces.push_back(new mesh_3_t::face_t(&v[0], &v[1], &v[2], &v[3]));
	faces.push_back(new mesh_3_t::face_t(&v[7], &v[6], &v[5], &v[4]));
	faces.push_back(new mesh_3_t::face_t(&v[0], &v[4], &v[5], &v[1]));
	faces.push_back(new mesh_3_t::face_t(&v[1], &v[5], &v[6], &v[2]));
	faces.push_back(new mesh_3_t::face_t(&v[2], &v[6], &v[7], &v[3]));
	faces.push_back(new mesh_3_t::face_t(&v[3], &v[7], &v[4], &v[0]));

	for (size_t i = 0; i < 6; ++i) {
		const float top = 0.0f, bottom = 1.0f;
		const float left = 0.f, right = 1.f;
		// const float left = i * (1.f / 6.f), right = (i + 1) * (1.f / 6.f);
		tex_coord_attr.setAttribute(faces[i], 0, tex_coord_t(left, top));
		tex_coord_attr.setAttribute(faces[i], 1, tex_coord_t(right, top));
		tex_coord_attr.setAttribute(faces[i], 2, tex_coord_t(right, bottom));
		tex_coord_attr.setAttribute(faces[i], 3, tex_coord_t(left, bottom));
		mtl_id_attr.setAttribute(faces[i], mtl_id);
	}

	return new mesh_3_t(faces);
}

struct cone_options
{
	float radius = 2.f, height = 2.f;
	uint32_t num_steps = 16;
	tmat<space::OBJECT, space::OBJECT> transform = tmat<space::OBJECT, space::OBJECT>();
};
mesh_3_t* textured_cone(tex_coord_attr_t& tex_coord_attr, mtl_id_attr_t& mtl_id_attr, GLuint mtl_id, const cone_options& options = {})
{
	assert(options.radius > 0.f);
	assert(options.height > 0.f);
	assert(options.num_steps > 0);

	const uint32_t STEPS = options.num_steps;
	const float DTHETA = (float)M_TWOPI / STEPS;
	std::vector<mesh_3_t::vertex_t*> vertices;
	vertices.reserve(STEPS + 1);
	for (uint32_t j = 0; j < STEPS; j++)
	{
		const float x = options.radius * cosf(j * DTHETA), z = options.radius * sinf(j * DTHETA);
		const float y = -options.height / 2;
		vertices.push_back(new mesh_3_t::vertex_t(hats2carve(point<space::OBJECT>(x, y, z).transform_copy(options.transform))));
	}
	vertices.push_back(new mesh_3_t::vertex_t(hats2carve(point<space::OBJECT>(0, options.height / 2, 0).transform_copy(options.transform))));

	std::vector<mesh_3_t::face_t*> faces;
	faces.reserve(STEPS + 1);
	mesh_3_t::face_t* face = new mesh_3_t::face_t(vertices.begin(), vertices.begin() + STEPS);
	faces.push_back(face);
	for (uint32_t i = 0; i < STEPS; i++)
	{
		const float u = (cosf(i * DTHETA) + 1) / 2, v = (sinf(i * DTHETA) + 1) / 2;
		tex_coord_attr.setAttribute(face, i, tex_coord_t(u, v));
	}
	for (uint32_t i = 0; i < STEPS; i++)
	{
		const uint32_t a = i, b = (i + 1) % STEPS, c = STEPS;
		face = new mesh_3_t::face_t(vertices[a], vertices[c], vertices[b]);
		tex_coord_attr.setAttribute(face, 0, tex_coord_t(1.f * i / STEPS, 0.0f));
		tex_coord_attr.setAttribute(face, 1, tex_coord_t(0.5f, 1.0f));
		tex_coord_attr.setAttribute(face, 2, tex_coord_t(1.f * (i + 1) / STEPS, 0.0f));
		faces.push_back(face);
	}

	for (mesh_3_t::face_t* face : faces)
	{
		mtl_id_attr.setAttribute(face, mtl_id);
	}

	return new mesh_3_t(faces);
}

struct torus_options
{
	float center_radius = 1.f, tube_radius = 1.f;
	uint32_t num_center_steps = 16, num_tube_steps = 16;
	carve::math::Matrix transform = carve::math::Matrix::IDENT();
};
mesh_3_t* textured_torus(tex_coord_attr_t& tex_coord_attr, mtl_id_attr_t& mtl_id_attr, GLuint mtl_id, const torus_options& options = {})
{
	assert(options.center_radius > 0.f);
	assert(options.tube_radius > 0.f);
	assert(options.num_center_steps > 0);
	assert(options.num_tube_steps > 0);

	const uint32_t CENTER_STEPS = options.num_center_steps;
	const uint32_t TUBE_STEPS = options.num_tube_steps;
	const float DTHETA = (float)M_TWOPI / CENTER_STEPS;
	const float DPHI = (float)M_TWOPI / TUBE_STEPS;
	std::vector<mesh_3_t::vertex_t*> vertices;
	vertices.reserve(TUBE_STEPS * CENTER_STEPS);
	for (uint32_t i = 0; i < CENTER_STEPS; i++)
	{
		const float ct = cosf(i * DTHETA), st = sinf(i * DTHETA);
		for (uint32_t j = 0; j < TUBE_STEPS; j++)
		{
			const float cp = cosf(j * DPHI), sp = sinf(j * DPHI);
			const float x = (options.center_radius + options.tube_radius * cp) * ct;
			const float y = options.tube_radius * sp;
			const float z = (options.center_radius + options.tube_radius * cp) * st;
			vertices.push_back(new mesh_3_t::vertex_t(options.transform * carve::geom::VECTOR(x, y, z)));
		}
	}

	std::vector<mesh_3_t::face_t*> faces;
	for (uint32_t i = 0; i < CENTER_STEPS; i++)
	{
		const uint32_t cur = i * TUBE_STEPS;
		const uint32_t next = ((i + 1) % CENTER_STEPS) * TUBE_STEPS;
		for (uint32_t j = 0; j < TUBE_STEPS; j++)
		{
			const uint32_t mj = (j + 1) % TUBE_STEPS;
			const uint32_t a = cur + j, b = cur + mj;
			const uint32_t c = next + j, d = next + mj;
			mesh_3_t::face_t* const face = new mesh_3_t::face_t(vertices[a], vertices[b], vertices[d], vertices[c]);
			tex_coord_attr.setAttribute(face, 0, tex_coord_t(1.f * i / CENTER_STEPS, 1 - 1.f * j / TUBE_STEPS));
			tex_coord_attr.setAttribute(face, 1, tex_coord_t(1.f * i / CENTER_STEPS, 1 - 1.f * (j + 1) / TUBE_STEPS));
			tex_coord_attr.setAttribute(face, 2, tex_coord_t((i + 1.f) / CENTER_STEPS, 1 - 1.f * (j + 1) / TUBE_STEPS));
			tex_coord_attr.setAttribute(face, 3, tex_coord_t((i + 1.f) / CENTER_STEPS, 1 - 1.f * j / TUBE_STEPS));
			mtl_id_attr.setAttribute(face, mtl_id);
			faces.push_back(face);
		}
	}

	return new mesh_3_t(faces);
}

void make_scene(carve::csg::CSG& csg, tex_coord_attr_t& tex_coord_attr, mtl_id_attr_t& mtl_id_attr, mesh_3_t*& out_mesh, std::unordered_map<GLuint, material_t>& out_mtls) {
	if (out_mesh) {
		delete out_mesh;
	}
	out_mtls.clear();


	/*GLuint WHITE_MTL_ID = 1;
	mtl_t mtl1;
	out_mtls.insert(std::make_pair(1, mtl1));
	mesh_3_t* box_a = textured_cube(tex_coord_attr, mtl_id_attr, 1);

	mtl_t mtl2;
	out_mtls.insert(std::make_pair(2, mtl2));
	mesh_3_t* box_b = textured_cube(tex_coord_attr, mtl_id_attr, 2, carve::math::Matrix::TRANS(0.5f, 0.5f, 0.5f));

	out_mesh = csg.compute(box_a, box_b, carve::csg::CSG::B_MINUS_A, nullptr, carve::csg::CSG::CLASSIFY_EDGE);

	delete box_a;
	delete box_b;*/


	material_t mtl1;
	out_mtls.insert(std::make_pair(1, mtl1));
	mesh_3_t* cyl = textured_cylinder(tex_coord_attr, mtl_id_attr, 1,
		{
			.top_radius = .5f,
			.bottom_radius = .5f,
			.transform = tmat_util::translation<space::OBJECT>(0, .5f, 0) * tmat_util::scale<space::OBJECT>(1.5f, 1.f, 1.5f)
		}
	);
	material_t mtl2;
	out_mtls.insert(std::make_pair(2, mtl2));
	mesh_3_t* box_b = textured_cuboid(tex_coord_attr, mtl_id_attr, 2,
		{
			.width = 3.f,
			.transform = tmat_util::translation<space::OBJECT>(0, -1.f, 0)
		}
	);
	mesh_3_t* box_a = textured_cuboid(tex_coord_attr, mtl_id_attr, 1,
		{
			.transform = tmat_util::translation<space::OBJECT>(1.5f, 0.f, 0)
		}
	);
	out_mesh = csg.compute(cyl, box_b, carve::csg::CSG::B_MINUS_A, nullptr, carve::csg::CSG::CLASSIFY_EDGE);
	// out_mesh = csg.compute(out_mesh, box_a, carve::csg::CSG::A_MINUS_B, nullptr, carve::csg::CSG::CLASSIFY_EDGE);

	mesh_3_t* cone = textured_cone(tex_coord_attr, mtl_id_attr, 1,
		{
			.radius = .5f,
			.height = 1.f,
			// .transform = carve::math::Matrix::TRANS(0.f, 1.5f, 0)
			.transform = tmat_util::translation<space::OBJECT>(0.f, 1.5f, 0)
		}
	);
	out_mesh = csg.compute(out_mesh, cone, carve::csg::CSG::UNION, nullptr, carve::csg::CSG::CLASSIFY_EDGE);

	mesh_3_t* tor = textured_torus(tex_coord_attr, mtl_id_attr, 2,
		{
			.tube_radius = .5f,
			.num_tube_steps = 8,
			.transform = carve::math::Matrix::TRANS(1.f, 0, 0)
		}
	);
	out_mesh = csg.compute(out_mesh, tor, carve::csg::CSG::A_MINUS_B, nullptr, carve::csg::CSG::CLASSIFY_EDGE);

	mesh_3_t* tor2 = textured_torus(tex_coord_attr, mtl_id_attr, 1,
		{
			.tube_radius = .5f,
			.num_tube_steps = 8,
			.transform = carve::math::Matrix::TRANS(3.f, 0, 0)
		}
	);
	out_mesh = csg.compute(out_mesh, tor2, carve::csg::CSG::UNION, nullptr, carve::csg::CSG::CLASSIFY_EDGE);

	delete tor;
	delete tor2;
	delete cone;
	delete cyl;
	delete box_b;
	delete box_a;
}

void compute_csg(std::unordered_map<GLuint, material_t>& out_mtls, std::unordered_map<GLuint, std::vector<GLfloat>>& out_vtxs_for_mtl) {
	out_mtls.clear();
	out_vtxs_for_mtl.clear();

	carve::csg::CSG csg;
	tex_coord_attr_t tex_coord_attr;
	tex_coord_attr.installHooks(csg);
	mtl_id_attr_t mtl_id_attr;
	mtl_id_attr.installHooks(csg);

	mesh_3_t* in_scene = nullptr;
	make_scene(csg, tex_coord_attr, mtl_id_attr, in_scene, out_mtls);
	for (auto it = out_mtls.begin(); it != out_mtls.end(); ++it) {
		out_vtxs_for_mtl.insert(std::make_pair(it->first, std::vector<GLfloat>()));
	}

	GLUtesselator* tess = gluNewTess();
	gluTessCallback(tess, GLU_TESS_BEGIN, (GLUTessCallback)tess_callback_begin);
	gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (GLUTessCallback)tess_callback_vertex_data);
	gluTessCallback(tess, GLU_TESS_EDGE_FLAG, (GLUTessCallback)tess_callback_edge_flag); // Edge flag forces only triangles
	gluTessCallback(tess, GLU_TESS_END, (GLUTessCallback)tess_callback_end);
	for (mesh_3_t::face_iter i = in_scene->faceBegin(); i != in_scene->faceEnd(); ++i) {
		mesh_3_t::face_t* f = *i;
		GLuint mtl_id = mtl_id_attr.getAttribute(f, 0);

		std::vector<tess_vtx> vtxs;
		for (mesh_3_t::face_t::edge_iter_t e = f->begin(); e != f->end(); ++e) {
			auto t = tex_coord_attr.getAttribute(f, e.idx());
			tess_vtx v;
			v.x = e->vert->v.x;
			v.y = e->vert->v.y;
			v.z = e->vert->v.z;
			v.u = t.u;
			v.v = t.v;
			v.target = &out_vtxs_for_mtl[mtl_id];
			vtxs.emplace_back(v);
		}

		gluTessBeginPolygon(tess, nullptr);
		gluTessBeginContour(tess);
		for (const tess_vtx& v : vtxs) {
			gluTessVertex(tess, (GLdouble*)&v, (GLvoid*)&v);
		}
		gluTessEndContour(tess);
		gluTessEndPolygon(tess);
	}
	gluDeleteTess(tess);

	delete in_scene;
}

static void GLAPIENTRY gl_dbg_msg_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}

void print_vtxs(const std::vector<GLfloat>& vtxs) {
	for (size_t i = 0; i < vtxs.size(); i += 5) {
		std::cout << vtxs[i] << " " << vtxs[i + 1] << " " << vtxs[i + 2] << " " << vtxs[i + 3] << " " << vtxs[i + 4] << "\n";
	}
	std::cout << "\n";
}

int main(int argc, char* argv[]) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#ifdef _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
	GLFWwindow* window = glfwCreateWindow(1280, 720, "PowerTranzphormR", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		return -1;
	}
#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(gl_dbg_msg_callback, 0);
#endif
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	GLuint vtx_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vtx_shader, 1, &VTX_SHADER_SRC, nullptr);
	glCompileShader(vtx_shader);
	GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag_shader, 1, &FRAG_SHADER_SRC, nullptr);
	glCompileShader(frag_shader);
	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, vtx_shader);
	glAttachShader(shader_program, frag_shader);
	glLinkProgram(shader_program);

	std::unordered_map<GLuint, material_t> mtls;
	std::unordered_map<GLuint, std::vector<GLfloat>> vtxs_for_mtl;

	auto time_start = std::chrono::high_resolution_clock::now();
	compute_csg(mtls, vtxs_for_mtl);
	auto time_finish = std::chrono::high_resolution_clock::now();
	std::cout << "CSG computed in " << std::chrono::duration_cast<std::chrono::milliseconds>(time_finish - time_start).count() << " milliseconds\n\n";

	std::unordered_map<GLuint, GLuint> vaos_for_mtl;
	std::unordered_map<GLuint, GLuint> vbos_for_mtl;
	for (auto it = vtxs_for_mtl.begin(); it != vtxs_for_mtl.end(); ++it) {
		std::cout << "Material " << it->first << " Vertex Buffer\n========================\n";
		// print_vtxs(it->second);

		GLuint vao = 0, vbo = 0;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, it->second.size() * sizeof(GLfloat), it->second.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		vaos_for_mtl.insert({ it->first, vao });
		vbos_for_mtl.insert({ it->first, vbo });
	}

	stbi_set_flip_vertically_on_load(true);
	std::unordered_map<GLuint, GLuint> texs_for_mtl;
	for (GLuint i = 1; i <= 2; ++i) {
		int tex_w = -1, tex_h = -1, tex_c = -1;
		stbi_uc* tex_data = stbi_load((std::string("res/") + std::to_string(i) + ".png").c_str(), &tex_w, &tex_h, &tex_c, 3);
		GLuint tex;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_w, tex_h, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);
		stbi_image_free(tex_data);
		texs_for_mtl.insert({ i, tex });
	}

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	globals::vp_prev_time = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		double now = glfwGetTime();
		globals::camera.update(now - globals::vp_prev_time);
		globals::vp_prev_time = now;

		int display_w = -1, display_h = -1;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0.4f, 0.6f, 0.9f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 proj_mtx = glm::perspective(90.0f * (float)M_PI / 180, static_cast<float>(display_w) / static_cast<float>(display_h), 0.001f, 1000.0f);
		glm::mat4 mvp_mtx = proj_mtx * globals::camera.view_mtx;

		for (auto it = vaos_for_mtl.begin(); it != vaos_for_mtl.end(); ++it) {
			glUseProgram(shader_program);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texs_for_mtl[it->first]);
			glUniform1i(glGetUniformLocation(shader_program, "u_tex"), 0);
			glUniform3f(glGetUniformLocation(shader_program, "u_col"), mtls[it->first].r, mtls[it->first].g, mtls[it->first].b);
			glUniformMatrix4fv(glGetUniformLocation(shader_program, "u_mvp"), 1, false, &mvp_mtx[0][0]);
			glBindVertexArray(it->second);
			glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vtxs_for_mtl[it->first].size() / 3);
		}
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
