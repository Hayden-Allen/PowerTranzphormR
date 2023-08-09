#include "pch.h"
#include "carve.h"

mesh_t* textured_cuboid(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const cuboid_options& options)
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
		// const float left = 0.f, right = 1.f;
		const float left = i * (1.f / 6.f), right = (i + 1) * (1.f / 6.f);
		tex_coord_attr.setAttribute(faces[i], 0, tex_coord_t(left, bottom));
		tex_coord_attr.setAttribute(faces[i], 1, tex_coord_t(right, bottom));
		tex_coord_attr.setAttribute(faces[i], 2, tex_coord_t(right, top));
		tex_coord_attr.setAttribute(faces[i], 3, tex_coord_t(left, top));
		mtl_id_attr.setAttribute(faces[i], mtl_id);
	}

	return new mesh_t(faces);
}

mesh_t* textured_cylinder(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const cylinder_options& options)
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

mesh_t* textured_cone(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const cone_options& options)
{
	assert(options.radius > 0.f);
	assert(options.height > 0.f);
	assert(options.num_steps > 0);

	const uint32_t STEPS = options.num_steps;
	const float DTHETA = (float)M_TWOPI / STEPS;
	std::vector<mesh_t::vertex_t*> vertices;
	vertices.reserve(STEPS + 1);
	for (uint32_t j = 0; j < STEPS; j++)
	{
		const float x = options.radius * cosf(j * DTHETA), z = options.radius * sinf(j * DTHETA);
		const float y = -options.height / 2;
		vertices.push_back(new vertex_t(hats2carve(point<space::OBJECT>(x, y, z).transform_copy(options.transform))));
	}
	vertices.push_back(new vertex_t(hats2carve(point<space::OBJECT>(0, options.height / 2, 0).transform_copy(options.transform))));

	std::vector<face_t*> faces;
	faces.reserve(STEPS + 1);
	face_t* face = new face_t(vertices.begin(), vertices.begin() + STEPS);
	faces.push_back(face);
	for (uint32_t i = 0; i < STEPS; i++)
	{
		const float u = (cosf(i * DTHETA) + 1) / 2, v = (sinf(i * DTHETA) + 1) / 2;
		tex_coord_attr.setAttribute(face, i, tex_coord_t(u, v));
	}
	for (uint32_t i = 0; i < STEPS; i++)
	{
		const uint32_t a = i, b = (i + 1) % STEPS, c = STEPS;
		face = new face_t(vertices[a], vertices[c], vertices[b]);
		tex_coord_attr.setAttribute(face, 0, tex_coord_t(1.f * i / STEPS, 0.0f));
		tex_coord_attr.setAttribute(face, 1, tex_coord_t(0.5f, 1.0f));
		tex_coord_attr.setAttribute(face, 2, tex_coord_t(1.f * (i + 1) / STEPS, 0.0f));
		faces.push_back(face);
	}

	for (face_t* face : faces)
	{
		mtl_id_attr.setAttribute(face, mtl_id);
	}

	return new mesh_t(faces);
}

mesh_t* textured_torus(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const torus_options& options)
{
	assert(options.center_radius > 0.f);
	assert(options.tube_radius > 0.f);
	assert(options.num_center_steps > 0);
	assert(options.num_tube_steps > 0);

	const uint32_t CENTER_STEPS = options.num_center_steps;
	const uint32_t TUBE_STEPS = options.num_tube_steps;
	const float DTHETA = (float)M_TWOPI / CENTER_STEPS;
	const float DPHI = (float)M_TWOPI / TUBE_STEPS;
	std::vector<vertex_t*> vertices;
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
			// vertices.push_back(new vertex_t(options.transform * carve::geom::VECTOR(x, y, z)));
			vertices.push_back(new vertex_t(hats2carve(hats::point<space::OBJECT>(x, y, z).transform_copy(options.transform))));
		}
	}

	std::vector<face_t*> faces;
	for (uint32_t i = 0; i < CENTER_STEPS; i++)
	{
		const uint32_t cur = i * TUBE_STEPS;
		const uint32_t next = ((i + 1) % CENTER_STEPS) * TUBE_STEPS;
		for (uint32_t j = 0; j < TUBE_STEPS; j++)
		{
			const uint32_t mj = (j + 1) % TUBE_STEPS;
			const uint32_t a = cur + j, b = cur + mj;
			const uint32_t c = next + j, d = next + mj;
			face_t* const face = new face_t(vertices[a], vertices[b], vertices[d], vertices[c]);
			tex_coord_attr.setAttribute(face, 0, tex_coord_t(1.f * i / CENTER_STEPS, 1 - 1.f * j / TUBE_STEPS));
			tex_coord_attr.setAttribute(face, 1, tex_coord_t(1.f * i / CENTER_STEPS, 1 - 1.f * (j + 1) / TUBE_STEPS));
			tex_coord_attr.setAttribute(face, 2, tex_coord_t((i + 1.f) / CENTER_STEPS, 1 - 1.f * (j + 1) / TUBE_STEPS));
			tex_coord_attr.setAttribute(face, 3, tex_coord_t((i + 1.f) / CENTER_STEPS, 1 - 1.f * j / TUBE_STEPS));
			mtl_id_attr.setAttribute(face, mtl_id);
			faces.push_back(face);
		}
	}

	return new mesh_t(faces);
}
