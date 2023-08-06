#pragma once
#include "pch.h"

struct tex_coord_t
{
	float u;
	float v;
	tex_coord_t() : u(0.0f), v(0.0f) {}
	tex_coord_t(float s, float t) : u(s), v(t) {}
};
static tex_coord_t operator*(double s, const tex_coord_t& t)
{
	return tex_coord_t(t.u * (float)s, t.v * (float)s);
}
static tex_coord_t& operator+=(tex_coord_t& t1, const tex_coord_t& t2)
{
	t1.u += t2.u;
	t1.v += t2.v;
	return t1;
}

struct material_t
{
	GLfloat r = 1.0f, g = 1.0f, b = 1.0f;
};

typedef carve::mesh::MeshSet<3> mesh_t;
typedef mesh_t::vertex_t vertex_t;
typedef mesh_t::face_t face_t;
typedef carve::interpolate::FaceVertexAttr<tex_coord_t> attr_tex_coord_t;
typedef carve::interpolate::FaceAttr<GLuint> attr_material_t;

template<space SPACE>
static carve::geom3d::Vector hats2carve(const point<SPACE>& p)
{
	return carve::geom::VECTOR(p.x, p.y, p.z);
}


struct cuboid_options
{
	float width = 2.f, height = 2.f, depth = 2.f;
	tmat<space::OBJECT, space::OBJECT> transform = tmat<space::OBJECT, space::OBJECT>();
};
static mesh_t* textured_cuboid(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const cuboid_options& options = {})
{
	assert(options.width > 0.f);
	assert(options.height > 0.f);
	assert(options.depth > 0.f);

	const float w = options.width / 2, h = options.height / 2, d = options.depth / 2;
	std::vector<vertex_t> v;
	v.reserve(8);
	// 0 - ftr
	v.push_back(vertex_t(hats2carve(point<space::OBJECT>(+w, +h, +d).transform_copy(options.transform))));
	// 1 - ftl
	v.push_back(vertex_t(hats2carve(point<space::OBJECT>(-w, +h, +d).transform_copy(options.transform))));
	// 2 - fbl
	v.push_back(vertex_t(hats2carve(point<space::OBJECT>(-w, -h, +d).transform_copy(options.transform))));
	// 3 - fbr
	v.push_back(vertex_t(hats2carve(point<space::OBJECT>(+w, -h, +d).transform_copy(options.transform))));
	// 4 - btr
	v.push_back(vertex_t(hats2carve(point<space::OBJECT>(+w, +h, -d).transform_copy(options.transform))));
	// 5 - btl
	v.push_back(vertex_t(hats2carve(point<space::OBJECT>(-w, +h, -d).transform_copy(options.transform))));
	// 6 - bbl
	v.push_back(vertex_t(hats2carve(point<space::OBJECT>(-w, -h, -d).transform_copy(options.transform))));
	// 7 - bbr
	v.push_back(vertex_t(hats2carve(point<space::OBJECT>(+w, -h, -d).transform_copy(options.transform))));

	std::vector<face_t*> faces;
	faces.reserve(6);
	// front
	faces.push_back(new face_t(&v[2], &v[3], &v[0], &v[1]));
	// left
	faces.push_back(new face_t(&v[6], &v[2], &v[1], &v[5]));
	// back
	faces.push_back(new face_t(&v[7], &v[6], &v[5], &v[4]));
	// right
	faces.push_back(new face_t(&v[3], &v[7], &v[4], &v[0]));
	// top
	faces.push_back(new face_t(&v[1], &v[0], &v[4], &v[5]));
	// bottom
	faces.push_back(new face_t(&v[6], &v[7], &v[3], &v[2]));

	for (size_t i = 0; i < 6; ++i) {
		const float top = 1.0f, bottom = 0.0f;
		const float left = 0.f, right = 1.f;
		// const float left = i * (1.f / 6.f), right = (i + 1) * (1.f / 6.f);
		tex_coord_attr.setAttribute(faces[i], 0, tex_coord_t(left, bottom));
		tex_coord_attr.setAttribute(faces[i], 1, tex_coord_t(right, bottom));
		tex_coord_attr.setAttribute(faces[i], 2, tex_coord_t(right, top));
		tex_coord_attr.setAttribute(faces[i], 3, tex_coord_t(left, top));
		mtl_id_attr.setAttribute(faces[i], mtl_id);
	}

	return new mesh_t(faces);
}


struct cylinder_options
{
	float top_radius = 1.f, bottom_radius = 1.f, height = 2.f;
	uint32_t num_steps = 16;
	tmat<space::OBJECT, space::OBJECT> transform = tmat<space::OBJECT, space::OBJECT>();
};
static mesh_t* textured_cylinder(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const cylinder_options& options = {})
{
	assert(options.num_steps > 0);
	assert(options.top_radius > 0.f);
	assert(options.bottom_radius > 0.f);
	assert(options.height > 0.f);

	const uint32_t STEPS = options.num_steps;
	const float DTHETA = (float)M_TWOPI / STEPS;
	const float radii[2] = { options.bottom_radius, options.top_radius };
	std::vector<mesh_t::vertex_t*> vertices;
	vertices.reserve(2 * STEPS);
	for (int i = 0; i < 2; i++)
	{
		const float r = radii[i];
		for (uint32_t j = 0; j < STEPS; j++)
		{
			const float x = r * cosf(j * DTHETA), z = r * sinf(j * DTHETA);
			const float y = (i * 2.f - 1.f) * options.height / 2;
			// vertices.push_back(new mesh_3_t::vertex_t(options.transform * carve::geom::VECTOR(x, y, z)));
			vertices.push_back(new vertex_t(hats2carve(point<space::OBJECT>(x, y, z).transform_copy(options.transform))));
		}
	}

	std::vector<face_t*> faces;
	faces.reserve(STEPS + 2);
	// bottom face is wound clockwise from below
	faces.push_back(new face_t(vertices.begin(), vertices.begin() + STEPS));
	// top face is wound clockwise from above
	faces.push_back(new face_t(vertices.rbegin(), vertices.rbegin() + STEPS));
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
		face_t* face = new face_t(vertices[a], vertices[c], vertices[d], vertices[b]);
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

	return new mesh_t(faces);
}


