#include "pch.h"
#include "carve.h"
#include "core/scene_ctx.h"

mesh_t* carve_clone(const mesh_t* const mesh, scene_ctx* const scene)
{
	auto& mtl_id_attr = scene->get_mtl_id_attr();
	auto& vert_attrs = scene->get_vert_attrs();

	std::unordered_map<const vertex_t*, vertex_t> verts_map;
	verts_map.reserve(mesh->vertex_storage.size());
	for (size_t i = 0; i < mesh->vertex_storage.size(); ++i)
	{
		const auto& v = mesh->vertex_storage[i];
		verts_map.insert({ &mesh->vertex_storage[i], vertex_t(carve::geom::VECTOR(v.v.x, v.v.y, v.v.z)) });
	}

	std::vector<face_t*> faces;
	for (mesh_t::const_face_iter i = mesh->faceBegin(); i != mesh->faceEnd(); ++i)
	{
		const mesh_t::face_t* const f = *i;
		const u32 mtl_id = mtl_id_attr.getAttribute(f, 0);

		std::vector<vertex_t*> verts;
		for (mesh_t::face_t::const_edge_iter_t e = f->begin(); e != f->end(); ++e)
		{
			verts.emplace_back(&verts_map[e->vert]);
		}
		mesh_t::face_t* const new_f = new mesh_t::face_t(verts.begin(), verts.end());
		mtl_id_attr.setAttribute(new_f, mtl_id);
		for (mesh_t::face_t::const_edge_iter_t e = f->begin(); e != f->end(); ++e)
		{
			const tex_coord_t uv0 = vert_attrs.uv0.getAttribute(f, e.idx());
			const tex_coord_t uv1 = vert_attrs.uv1.getAttribute(f, e.idx());
			const tex_coord_t uv2 = vert_attrs.uv2.getAttribute(f, e.idx());
			const tex_coord_t uv3 = vert_attrs.uv3.getAttribute(f, e.idx());
			const f64 w0 = vert_attrs.w0.getAttribute(f, e.idx());
			const f64 w1 = vert_attrs.w1.getAttribute(f, e.idx());
			const f64 w2 = vert_attrs.w2.getAttribute(f, e.idx());
			const f64 w3 = vert_attrs.w3.getAttribute(f, e.idx());
			const color_t& color = vert_attrs.color.getAttribute(f, e.idx());
			vert_attrs.uv0.setAttribute(new_f, e.idx(), uv0);
			vert_attrs.uv1.setAttribute(new_f, e.idx(), uv1);
			vert_attrs.uv2.setAttribute(new_f, e.idx(), uv2);
			vert_attrs.uv3.setAttribute(new_f, e.idx(), uv3);
			vert_attrs.w0.setAttribute(new_f, e.idx(), w0);
			vert_attrs.w1.setAttribute(new_f, e.idx(), w1);
			vert_attrs.w2.setAttribute(new_f, e.idx(), w2);
			vert_attrs.w3.setAttribute(new_f, e.idx(), w3);
			vert_attrs.color.setAttribute(new_f, e.idx(), color);
		}
		faces.emplace_back(new_f);
	}

	mesh_t* const clone = new mesh_t(faces);
	return clone;
}

mesh_t* textured_cuboid(carve_vert_attrs& vert_attrs, attr_material_t& mtl_id_attr, const GLuint mtl_id, const cuboid_options& options)
{
	// generate vertices for cube centered at local origin
	const f32 w = .5f, h = .5f, d = .5f;
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

	// facez are wound cw
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

	for (s32 i = 0; i < 6; ++i)
	{
		vert_attrs.set_attribute(faces[i], 0, options, tex_coord_t(0.f, 0.f));
		vert_attrs.set_attribute(faces[i], 1, options, tex_coord_t(1.f, 0.f));
		vert_attrs.set_attribute(faces[i], 2, options, tex_coord_t(1.f, 1.f));
		vert_attrs.set_attribute(faces[i], 3, options, tex_coord_t(0.f, 1.f));
		mtl_id_attr.setAttribute(faces[i], mtl_id);
	}

	return new mesh_t(faces);
}

mesh_t* textured_cylinder(carve_vert_attrs& vert_attrs, attr_material_t& mtl_id_attr, const GLuint mtl_id, const cylinder_options& options)
{
	assert(options.num_steps > 0);
	assert(options.top_radius > 0.f);
	assert(options.bottom_radius > 0.f);

	// number of vertical strips to generate
	const u32 STEPS = options.num_steps;
	// angular resolution of each vertical strip
	const f32 DTHETA = c::TWO_PI / STEPS;
	const f32 radii[2] = { options.bottom_radius, options.top_radius };
	std::vector<vertex_t> raw_vertices;
	std::vector<vertex_t*> vertices;
	vertices.reserve(2 * STEPS);
	// vertices are the circles defining the cylinder
	for (int i = 0; i < 2; i++)
	{
		// top and bottom circle can have different radii
		const f32 r = radii[i];
		// generate a circle with current radius
		for (u32 j = 0; j < STEPS; j++)
		{
			// generate clockwise from x-axis
			const f32 x = r * cosf(j * DTHETA), z = r * sinf(j * DTHETA);
			// make cylinder vertically centered on the local origin
			const f32 y = (i * 2.f - 1.f) * .5f;
			raw_vertices.emplace_back(hats2carve(point<space::OBJECT>(x, y, z).transform_copy(options.transform)));
		}
	}
	for (u64 i = 0; i < raw_vertices.size(); i++)
		vertices.push_back(raw_vertices.data() + i);

	std::vector<face_t*> faces;
	faces.reserve(STEPS + 2);
	// circular faces
	// bottom face is wound clockwise from below
	faces.push_back(new face_t(vertices.begin(), vertices.begin() + STEPS));
	// top face is wound clockwise from above
	faces.push_back(new face_t(vertices.rbegin(), vertices.rbegin() + STEPS));
	for (const face_t* const face : faces)
	{
		for (u32 i = 0; i < STEPS; i++)
		{
			// map each vertex to where it lies within the texture
			// (don't stretch rectangle texture to fit circle, just take subtexture that fits)
			const f32 u = (cosf(i * DTHETA) + 1) / 2, v = (sinf(i * DTHETA) + 1) / 2;
			vert_attrs.set_attribute(face, i, options, tex_coord_t(u, v));
		}
	}
	// vertical strips
	for (u32 i = 0; i < STEPS; i++)
	{
		const u32 a = i, b = (i + 1) % STEPS;
		const u32 c = i + STEPS, d = b + STEPS;
		// wound clockwise
		face_t* face = new face_t(vertices[a], vertices[c], vertices[d], vertices[b]);
		vert_attrs.set_attribute(face, 0, options, tex_coord_t(1.f * i / STEPS, 0.f));
		vert_attrs.set_attribute(face, 1, options, tex_coord_t(1.f * i / STEPS, 1.f));
		vert_attrs.set_attribute(face, 2, options, tex_coord_t(1.f * (i + 1.f) / STEPS, 1.f));
		vert_attrs.set_attribute(face, 3, options, tex_coord_t(1.f * (i + 1.f) / STEPS, 0.f));
		faces.push_back(face);
	}

	for (const face_t* const face : faces)
		mtl_id_attr.setAttribute(face, mtl_id);

	return new mesh_t(faces);
}

mesh_t* textured_cone(carve_vert_attrs& vert_attrs, attr_material_t& mtl_id_attr, const GLuint mtl_id, const cone_options& options)
{
	assert(options.num_steps > 0);

	// generate circular base of the cone
	const u32 STEPS = options.num_steps;
	const f32 DTHETA = c::TWO_PI / STEPS;
	std::vector<vertex_t> raw_vertices;
	std::vector<vertex_t*> vertices;
	vertices.reserve(STEPS + 1);
	for (u32 j = 0; j < STEPS; j++)
	{
		const f32 x = .5f * cosf(j * DTHETA), z = .5f * sinf(j * DTHETA);
		// make cone vertically centered on the origin
		const f32 y = -.5f;
		raw_vertices.emplace_back(hats2carve(point<space::OBJECT>(x, y, z).transform_copy(options.transform)));
	}
	// top point of cone
	raw_vertices.emplace_back(hats2carve(point<space::OBJECT>(0, .5f, 0).transform_copy(options.transform)));
	for (u64 i = 0; i < raw_vertices.size(); i++)
		vertices.push_back(raw_vertices.data() + i);

	std::vector<face_t*> faces;
	faces.reserve(STEPS + 1);
	// single face for cirular base of cone (wound clockwise)
	face_t* face = new face_t(vertices.begin(), vertices.begin() + STEPS);
	faces.push_back(face);
	// same as for textured_cylinder, take subtexture that fits on the circle
	for (uint32_t i = 0; i < STEPS; i++)
	{
		const f32 u = (cosf(i * DTHETA) + 1) / 2, v = (sinf(i * DTHETA) + 1) / 2;
		vert_attrs.set_attribute(face, i, options, tex_coord_t(u, v));
	}
	// generate vertical strips for cone
	for (u32 i = 0; i < STEPS; i++)
	{
		const u32 a = i, b = (i + 1) % STEPS, c = STEPS;
		face = new face_t(vertices[a], vertices[c], vertices[b]);
		vert_attrs.set_attribute(face, 0, options, tex_coord_t(1.f * i / STEPS, 0.f));
		vert_attrs.set_attribute(face, 1, options, tex_coord_t(.5f, 1.f));
		vert_attrs.set_attribute(face, 2, options, tex_coord_t(1.f * (i + 1) / STEPS, 0.f));
		faces.push_back(face);
	}

	for (const face_t* const face : faces)
		mtl_id_attr.setAttribute(face, mtl_id);

	return new mesh_t(faces);
}

mesh_t* textured_torus(carve_vert_attrs& vert_attrs, attr_material_t& mtl_id_attr, const GLuint mtl_id, const torus_options& options)
{
	assert(options.center_radius > 0.f);
	assert(options.tube_radius > 0.f);
	assert(options.num_center_steps > 2);
	assert(options.num_tube_steps > 2);

	// number of circles to generate around the center of the torus
	const u32 CENTER_STEPS = options.num_center_steps;
	// number of steps for each circle
	const u32 TUBE_STEPS = options.num_tube_steps;
	// torus resolution
	const f32 DTHETA = c::TWO_PI / CENTER_STEPS;
	// resolution of each circle
	const f32 DPHI = c::TWO_PI / TUBE_STEPS;
	std::vector<vertex_t> raw_vertices;
	std::vector<vertex_t*> vertices;
	vertices.reserve(TUBE_STEPS * CENTER_STEPS);
	for (u32 i = 0; i < CENTER_STEPS; i++)
	{
		const f32 ct = cosf(i * DTHETA), st = sinf(i * DTHETA);
		// generate circle rotated about the y-axis by `theta` and translated appropriately
		for (u32 j = 0; j < TUBE_STEPS; j++)
		{
			const f32 cp = cosf(j * DPHI), sp = sinf(j * DPHI);
			// rotate point around the circle's origin, which is relative to `theta`
			const f32 x = (options.center_radius + options.tube_radius * cp) * ct;
			const f32 y = options.tube_radius * sp;
			const f32 z = (options.center_radius + options.tube_radius * cp) * st;
			raw_vertices.emplace_back(hats2carve(hats::point<space::OBJECT>(x, y, z).transform_copy(options.transform)));
		}
	}
	for (u64 i = 0; i < raw_vertices.size(); i++)
		vertices.push_back(raw_vertices.data() + i);

	std::vector<face_t*> faces;
	// treat each pair of circles (each step of the tube) as a cylinder, and generate `TUBE_STEPS` vertical strips
	for (u32 i = 0; i < CENTER_STEPS; i++)
	{
		const u32 cur = i * TUBE_STEPS;
		const u32 next = ((i + 1) % CENTER_STEPS) * TUBE_STEPS;
		for (u32 j = 0; j < TUBE_STEPS; j++)
		{
			const u32 mj = (j + 1) % TUBE_STEPS;
			const u32 a = cur + j, b = cur + mj;
			const u32 c = next + j, d = next + mj;
			face_t* const face = new face_t(vertices[a], vertices[b], vertices[d], vertices[c]);
			vert_attrs.set_attribute(face, 0, options, tex_coord_t(1.f * i / CENTER_STEPS, (1 - 1.f * j / TUBE_STEPS)));
			vert_attrs.set_attribute(face, 1, options, tex_coord_t(1.f * i / CENTER_STEPS, (1 - 1.f * (j + 1) / TUBE_STEPS)));
			vert_attrs.set_attribute(face, 2, options, tex_coord_t((i + 1.f) / CENTER_STEPS, (1 - 1.f * (j + 1) / TUBE_STEPS)));
			vert_attrs.set_attribute(face, 3, options, tex_coord_t((i + 1.f) / CENTER_STEPS, (1 - 1.f * j / TUBE_STEPS)));
			mtl_id_attr.setAttribute(face, mtl_id);
			faces.push_back(face);
		}
	}

	return new mesh_t(faces);
}

mesh_t* textured_ellipsoid(carve_vert_attrs& vert_attrs, attr_material_t& mtl_id_attr, const GLuint mtl_id, const ellipsoid_options& options)
{
	assert(options.num_horizontal_steps > 2);
	assert(options.num_vertical_steps > 3);

	const u32 nv = options.num_vertical_steps, nh = options.num_horizontal_steps;
	const f32 DPHI = c::PI / nv, DTHETA = c::TWO_PI / nh;
	std::vector<vertex_t> raw_vertices;
	std::vector<vertex_t*> vertices;
	vertices.reserve(nh * nv + 2);
	// [1, nv-1] because poles are added separately
	// work from bottom to top
	for (u32 iy = 1; iy < nv; iy++)
	{
		const f32 phi = iy * DPHI;
		const f32 cp = cos(phi), sp = sin(phi);
		const f32 y = .5f * sin(phi - c::PI / 2);
		// generate ellipse for current vertical step
		for (u32 ix = 0; ix < nh; ix++)
		{
			const f32 theta = ix * DTHETA;
			const f32 ct = cos(theta), st = sin(theta);
			const f32 x = .5f * sp * ct;
			// due to imprecision, values of `ct` that should be 0 can cause the radical used to calculate `z` can be negative
			// when `ct` is 0, `z` should also be 0, so make this the default value
			f32 z = 0.f;
			if (abs(abs(ct) - 1) > c::EPSILON)
			{
				const f32 sp2 = sp * sp, ct2 = ct * ct;
				const f32 tmp = sin(phi - c::PI / 2) * sin(phi - c::PI / 2);

				const f32 rz2 = 1.f;
				const f32 rad = 1 - ct2 * sp2 - tmp;
				z = .5f * u::sign(st) * sqrt(rad);
			}
			// take sign of each axis (if zero, choose random sign)
			const s32 sx = u::sign(x) != 0 ? u::sign(x) : u::sign(u::rand(-1.f, 1.f));
			const s32 sy = u::sign(y) != 0 ? u::sign(y) : u::sign(u::rand(-1.f, 1.f));
			const s32 sz = u::sign(z) != 0 ? u::sign(z) : u::sign(u::rand(-1.f, 1.f));
			// random noise within threshold, in direction of current point
			const f32 nx = sx * u::rand(0.f, options.noise);
			const f32 ny = sy * u::rand(0.f, options.noise);
			const f32 nz = sz * u::rand(0.f, options.noise);
			raw_vertices.emplace_back(hats2carve(hats::point<space::OBJECT>(x + nx, y + ny, z + nz).transform_copy(options.transform)));
		}
	}
	// poles
	raw_vertices.emplace_back(hats2carve(hats::point<space::OBJECT>(0, -.5f, 0).transform_copy(options.transform)));
	raw_vertices.emplace_back(hats2carve(hats::point<space::OBJECT>(0, .5f, 0).transform_copy(options.transform)));
	for (u64 i = 0; i < raw_vertices.size(); i++)
		vertices.push_back(raw_vertices.data() + i);

	std::vector<face_t*> faces;
	// poles are comprised of triangles from each vertical strip
	// bottom pole
	for (u32 i = 0; i < nh; i++)
	{
		const u32 a = i, b = (i + 1) % nh;
		const u32 c = MGL_CAST(u32, vertices.size() - 2);
		face_t* const face = new face_t(vertices[a], vertices[b], vertices[c]);
		vert_attrs.set_attribute(face, 0, options, tex_coord_t(1.f * i / nh, 1.f / nv));
		vert_attrs.set_attribute(face, 1, options, tex_coord_t((i + 1.f) / nh, 1.f / nv));
		vert_attrs.set_attribute(face, 2, options, tex_coord_t((i + .5f) / nh, 0.f));
		mtl_id_attr.setAttribute(face, mtl_id);
		faces.push_back(face);
	}
	// top pole
	for (u32 i = 0; i < nh; i++)
	{
		const u32 off = nh * (nv - 2);
		const u32 a = i + off, b = (i + 1) % nh + off;
		const u32 c = MGL_CAST(u32, vertices.size() - 1);
		face_t* const face = new face_t(vertices[a], vertices[c], vertices[b]);
		vert_attrs.set_attribute(face, 0, options, tex_coord_t(1.f * i / nh, (nv - 1.f) / nv));
		vert_attrs.set_attribute(face, 1, options, tex_coord_t((i + .5f) / nh, 1.f));
		vert_attrs.set_attribute(face, 2, options, tex_coord_t((i + 1.f) / nh, (nv - 1.f) / nv));
		mtl_id_attr.setAttribute(face, mtl_id);
		faces.push_back(face);
	}
	// body is comprised of vertical strips made of quads
	for (u32 iy = 1; iy < nv - 1; iy++)
	{
		const u32 cur_off = (iy - 1) * nh;
		const u32 next_off = iy * nh;
		const f32 cty = 1.f * iy / nv, nty = 1.f * (iy + 1) / nv;
		for (u32 ix = 0; ix < nh; ix++)
		{
			const f32 ctx = 1.f * ix / nh, ntx = 1.f * (ix + 1) / nh;
			const u32 a = ix + cur_off, b = (ix + 1) % nh + cur_off;
			const u32 c = ix + next_off, d = (ix + 1) % nh + next_off;
			face_t* const face = new face_t(vertices[a], vertices[c], vertices[d], vertices[b]);
			vert_attrs.set_attribute(face, 0, options, tex_coord_t(ctx, cty));
			vert_attrs.set_attribute(face, 1, options, tex_coord_t(ctx, nty));
			vert_attrs.set_attribute(face, 2, options, tex_coord_t(ntx, nty));
			vert_attrs.set_attribute(face, 3, options, tex_coord_t(ntx, cty));
			mtl_id_attr.setAttribute(face, mtl_id);
			faces.push_back(face);
		}
	}

	return new mesh_t(faces);
}

mesh_t* textured_heightmap(carve_vert_attrs& vert_attrs, attr_material_t& mtl_id_attr, const GLuint mtl_id, const heightmap_options& options)
{
	assert(false);
	return nullptr;

	/*
	assert(options.u_scale > 0.f);
	assert(options.v_scale > 0.f);
	assert(options.width_steps > 0.f);
	assert(options.depth_steps > 0.f);

	const mgl::texture2d_rgb_u8* const map = g::texlib->get_or_load(options.map_path);
	const u32 mw = map->get_width(), mh = map->get_height();
	const u32 x_steps = options.width_steps == 0 ? mw : options.width_steps;
	const u32 z_steps = options.depth_steps == 0 ? mh : options.depth_steps;
	const f32 x_step = 1.f / (x_steps - 1);
	const f32 z_step = 1.f / (z_steps - 1);
	std::vector<vertex_t> raw_vertices;
	std::vector<vertex_t*> vertices;
	vertices.reserve(x_steps * z_steps);
	// generate plane with one vertex for each pixel in `map`
	// y-axis of texture maps to z-axis of plane
	for (u32 iz = 0; iz < z_steps; iz++)
	{
		const f32 z = iz * z_step - .5f;
		// x-axis of texture maps to x-axis of plane
		for (u32 ix = 0; ix < x_steps; ix++)
		{
			const f32 x = ix * x_step - .5f;
			raw_vertices.emplace_back(hats2carve(hats::point<space::OBJECT>(x, 0, z).transform_copy(options.transform)));
		}
	}
	for (u64 i = 0; i < raw_vertices.size(); i++)
		vertices.push_back(raw_vertices.data() + i);

	std::vector<face_t*> faces;
	for (u32 iz = 0; iz < z_steps - 1; iz++)
	{
		for (u32 ix = 0; ix < x_steps - 1; ix++)
		{
			// square with current vertex (ix, iz) as bottom left
			const u32 bl = iz * x_steps + ix, br = bl + 1;
			const u32 tl = (iz + 1) * x_steps + ix, tr = tl + 1;
			// bottom right triangle
			face_t* face = new face_t(vertices[bl], vertices[tr], vertices[br]);
			vert_attrs.set_attribute(face, 0, tex_coord_t(options.u_scale * ix / x_steps, options.v_scale * (1.f - 1.f * iz / z_steps)));
			vert_attrs.set_attribute(face, 1, tex_coord_t(options.u_scale * (ix + 1) / x_steps, options.v_scale * (1.f - 1.f * (iz + 1) / z_steps)));
			vert_attrs.set_attribute(face, 2, tex_coord_t(options.u_scale * (ix + 1) / x_steps, options.v_scale * (1.f - 1.f * iz / z_steps)));
			mtl_id_attr.setAttribute(face, mtl_id);
			faces.push_back(face);
			// top left triangle
			face = new face_t(vertices[bl], vertices[tl], vertices[tr]);
			vert_attrs.set_attribute(face, 0, tex_coord_t(options.u_scale * ix / x_steps, options.u_scale * (1.f - 1.f * iz / z_steps)));
			vert_attrs.set_attribute(face, 1, tex_coord_t(options.u_scale * ix / x_steps, options.u_scale * (1.f - 1.f * (iz + 1) / z_steps)));
			vert_attrs.set_attribute(face, 2, tex_coord_t(options.u_scale * (ix + 1) / x_steps, options.u_scale * (1.f - 1.f * (iz + 1) / z_steps)));
			mtl_id_attr.setAttribute(face, mtl_id);
			faces.push_back(face);
		}
	}

	return new mesh_t(faces);
	*/
}
