#include "pch.h"
#include "carve.h"
#include "core/scene_ctx.h"

mesh_t* carve_clone(const mesh_t* const mesh, scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& inv_mat)
{
	auto& mtl_id_attr = scene->get_mtl_id_attr();
	auto& vert_attrs = scene->get_vert_attrs();

	std::unordered_map<const vertex_t*, vertex_t> verts_map;
	verts_map.reserve(mesh->vertex_storage.size());
	for (size_t i = 0; i < mesh->vertex_storage.size(); ++i)
	{
		const auto& v = mesh->vertex_storage[i];
		verts_map.insert({
			&mesh->vertex_storage[i],
			u::hats2carve(hats::point<space::WORLD>(v.v.x, v.v.y, v.v.z).transform_copy(inv_mat)),
		});
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
	v.emplace_back(carve::geom::VECTOR(+w, +h, +d));
	// 1 - ftl
	v.emplace_back(carve::geom::VECTOR(-w, +h, +d));
	// 2 - fbl
	v.emplace_back(carve::geom::VECTOR(-w, -h, +d));
	// 3 - fbr
	v.emplace_back(carve::geom::VECTOR(+w, -h, +d));
	// 4 - btr
	v.emplace_back(carve::geom::VECTOR(+w, +h, -d));
	// 5 - btl
	v.emplace_back(carve::geom::VECTOR(-w, +h, -d));
	// 6 - bbl
	v.emplace_back(carve::geom::VECTOR(-w, -h, -d));
	// 7 - bbr
	v.emplace_back(carve::geom::VECTOR(+w, -h, -d));

	// facez are wound cw
	std::vector<face_t*> faces;
	faces.reserve(6);
	// front
	// faces.push_back(new face_t(&v[2], &v[3], &v[0], &v[1]));
	faces.push_back(new face_t(&v[2], &v[3], &v[0]));
	faces.push_back(new face_t(&v[2], &v[0], &v[1]));
	// left
	// faces.push_back(new face_t(&v[6], &v[2], &v[1], &v[5]));
	faces.push_back(new face_t(&v[6], &v[2], &v[1]));
	faces.push_back(new face_t(&v[6], &v[1], &v[5]));
	// back
	// faces.push_back(new face_t(&v[7], &v[6], &v[5], &v[4]));
	faces.push_back(new face_t(&v[7], &v[6], &v[5]));
	faces.push_back(new face_t(&v[7], &v[5], &v[4]));
	// right
	// faces.push_back(new face_t(&v[3], &v[7], &v[4], &v[0]));
	faces.push_back(new face_t(&v[3], &v[7], &v[4]));
	faces.push_back(new face_t(&v[3], &v[4], &v[0]));
	// top
	// faces.push_back(new face_t(&v[1], &v[0], &v[4], &v[5]));
	faces.push_back(new face_t(&v[1], &v[0], &v[4]));
	faces.push_back(new face_t(&v[1], &v[4], &v[5]));
	// bottom
	// faces.push_back(new face_t(&v[6], &v[7], &v[3], &v[2]));
	faces.push_back(new face_t(&v[6], &v[7], &v[3]));
	faces.push_back(new face_t(&v[6], &v[3], &v[2]));

	for (s32 i = 0; i < faces.size(); ++i)
	{
		vert_attrs.set_attribute(faces[i], 0, options, tex_coord_t(0.f, 0.f));
		if (i % 2 == 0)
			vert_attrs.set_attribute(faces[i], 1, options, tex_coord_t(1.f, 0.f));
		else
			vert_attrs.set_attribute(faces[i], 1, options, tex_coord_t(1.f, 1.f));
		if (i % 2 == 0)
			vert_attrs.set_attribute(faces[i], 2, options, tex_coord_t(1.f, 1.f));
		else
			vert_attrs.set_attribute(faces[i], 2, options, tex_coord_t(0.f, 1.f));

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
	// vertices on circle boundaries, vertices at circle centers
	const u32 num_verts = (STEPS * 2) + 2;
	std::vector<vertex_t*> vertices;
	vertices.reserve(2 * STEPS + 2);
	// vertices are the circles defining the cylinder
	for (int i = 0; i < 2; i++)
	{
		const f32 y = (i * 2.f - 1.f) * .5f;
		// top and bottom circle can have different radii
		const f32 r = radii[i];
		// generate a circle with current radius
		for (u32 j = 0; j < STEPS; j++)
		{
			// generate clockwise from x-axis
			const f32 x = r * cosf(j * DTHETA), z = r * sinf(j * DTHETA);
			// make cylinder vertically centered on the local origin
			raw_vertices.emplace_back(carve::geom::VECTOR(x, y, z));
		}
		raw_vertices.emplace_back(carve::geom::VECTOR(0, y, 0));
	}
	for (u64 i = 0; i < raw_vertices.size(); i++)
		vertices.push_back(raw_vertices.data() + i);

	// const u32 num_faces = STEPS + 2;
	// wall tris, top face tris, bottom face tris
	const u32 num_faces = (STEPS * 2) + (STEPS) + (STEPS);
	std::vector<face_t*> faces;
	faces.reserve(num_faces);
	// circular faces
	// bottom face is wound clockwise from below
	for (u32 j = 0; j < 2; j++)
	{
		const u32 offset = j * (STEPS + 1);
		for (u32 i = 0; i < STEPS; i++)
		{
			const u32 ia = STEPS + offset;
			u32 ib = i + offset, ic = (i + 1) % STEPS + offset;
			// ordering different for top/bottom face to maintain cw
			if (j % 2 == 1)
				std::swap(ib, ic);

			face_t* const f = new face_t(vertices[ia], vertices[ib], vertices[ic]);
			faces.push_back(f);

			f32 ub = (cosf(i * DTHETA) + 1) / 2, vb = (sinf(i * DTHETA) + 1) / 2;
			f32 uc = (cosf(((i + 1) % STEPS) * DTHETA) + 1) / 2, vc = (sinf(((i + 1) % STEPS) * DTHETA) + 1) / 2;
			if (j % 2 == 1)
			{
				std::swap(ub, uc);
				std::swap(vb, vc);
			}
			vert_attrs.set_attribute(f, 0, options, tex_coord_t(.5f, .5f));
			vert_attrs.set_attribute(f, 1, options, tex_coord_t(ub, vb));
			vert_attrs.set_attribute(f, 2, options, tex_coord_t(uc, vc));
		}
	}
	// vertical strips
	for (u32 i = 0; i < STEPS; i++)
	{
		const u32 a = i, b = (i + 1) % STEPS;
		const u32 c = i + STEPS + 1, d = b + STEPS + 1;
		// wound clockwise
		face_t* face1 = new face_t(vertices[a], vertices[c], vertices[d]);
		vert_attrs.set_attribute(face1, 0, options, tex_coord_t(1.f * i / STEPS, 0.f));
		vert_attrs.set_attribute(face1, 1, options, tex_coord_t(1.f * i / STEPS, 1.f));
		vert_attrs.set_attribute(face1, 2, options, tex_coord_t(1.f * (i + 1.f) / STEPS, 1.f));
		faces.push_back(face1);
		face_t* face2 = new face_t(vertices[a], vertices[d], vertices[b]);
		vert_attrs.set_attribute(face2, 0, options, tex_coord_t(1.f * i / STEPS, 0.f));
		vert_attrs.set_attribute(face2, 1, options, tex_coord_t(1.f * (i + 1.f) / STEPS, 1.f));
		vert_attrs.set_attribute(face2, 2, options, tex_coord_t(1.f * (i + 1.f) / STEPS, 0.f));
		faces.push_back(face2);
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
	// base verts + circle center + point
	const u32 num_verts = STEPS + 2;
	std::vector<vertex_t*> vertices;
	vertices.reserve(num_verts);
	for (u32 j = 0; j < STEPS; j++)
	{
		const f32 x = .5f * cosf(j * DTHETA), z = .5f * sinf(j * DTHETA);
		// make cone vertically centered on the origin
		const f32 y = -.5f;
		raw_vertices.emplace_back(carve::geom::VECTOR(x, y, z));
	}
	// circle center
	raw_vertices.emplace_back(carve::geom::VECTOR(0, -.5f, 0));
	// top point of cone
	raw_vertices.emplace_back(carve::geom::VECTOR(0, .5f, 0));
	for (u64 i = 0; i < raw_vertices.size(); i++)
		vertices.push_back(raw_vertices.data() + i);

	// vertical faces + base faces
	const u32 num_faces = STEPS * 2;
	std::vector<face_t*> faces;
	faces.reserve(num_faces);
	// single face for cirular base of cone (wound clockwise)
	for (u32 i = 0; i < STEPS; i++)
	{
		const u32 ia = STEPS;
		const u32 ib = i, ic = (i + 1) % STEPS;
		face_t* const face = new face_t(vertices[ia], vertices[ib], vertices[ic]);
		faces.push_back(face);

		const f32 ub = (cosf(i * DTHETA) + 1) / 2, vb = (sinf(i * DTHETA) + 1) / 2;
		const f32 uc = (cosf(((i + 1) % STEPS) * DTHETA) + 1) / 2, vc = (sinf(((i + 1) % STEPS) * DTHETA) + 1) / 2;
		vert_attrs.set_attribute(face, 0, options, tex_coord_t(.5f, .5f));
		vert_attrs.set_attribute(face, 1, options, tex_coord_t(uc, vc));
		vert_attrs.set_attribute(face, 2, options, tex_coord_t(ub, vb));
	}

	//  generate vertical strips for cone
	for (u32 i = 0; i < STEPS; i++)
	{
		const u32 a = i, b = (i + 1) % STEPS, c = STEPS + 1;
		face_t* const face = new face_t(vertices[a], vertices[c], vertices[b]);
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
	const u32 num_verts = TUBE_STEPS * CENTER_STEPS;
	std::vector<vertex_t*> vertices;
	vertices.reserve(num_verts);
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
			raw_vertices.emplace_back(carve::geom::VECTOR(x, y, z));
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
			face_t* const face1 = new face_t(vertices[a], vertices[b], vertices[d]);
			vert_attrs.set_attribute(face1, 0, options, tex_coord_t(1.f * i / CENTER_STEPS, (1 - 1.f * j / TUBE_STEPS)));
			vert_attrs.set_attribute(face1, 1, options, tex_coord_t(1.f * i / CENTER_STEPS, (1 - 1.f * (j + 1) / TUBE_STEPS)));
			vert_attrs.set_attribute(face1, 2, options, tex_coord_t((i + 1.f) / CENTER_STEPS, (1 - 1.f * (j + 1) / TUBE_STEPS)));
			mtl_id_attr.setAttribute(face1, mtl_id);
			faces.push_back(face1);
			face_t* const face2 = new face_t(vertices[a], vertices[d], vertices[c]);
			vert_attrs.set_attribute(face2, 0, options, tex_coord_t(1.f * i / CENTER_STEPS, (1 - 1.f * j / TUBE_STEPS)));
			vert_attrs.set_attribute(face2, 1, options, tex_coord_t((i + 1.f) / CENTER_STEPS, (1 - 1.f * (j + 1) / TUBE_STEPS)));
			vert_attrs.set_attribute(face2, 2, options, tex_coord_t((i + 1.f) / CENTER_STEPS, (1 - 1.f * j / TUBE_STEPS)));
			mtl_id_attr.setAttribute(face2, mtl_id);
			faces.push_back(face2);
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
			raw_vertices.emplace_back(carve::geom::VECTOR(x + nx, y + ny, z + nz));
		}
	}
	// poles
	raw_vertices.emplace_back(carve::geom::VECTOR(0, -.5f, 0));
	raw_vertices.emplace_back(carve::geom::VECTOR(0, .5f, 0));
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
			face_t* const face1 = new face_t(vertices[a], vertices[c], vertices[d]);
			vert_attrs.set_attribute(face1, 0, options, tex_coord_t(ctx, cty));
			vert_attrs.set_attribute(face1, 1, options, tex_coord_t(ctx, nty));
			vert_attrs.set_attribute(face1, 2, options, tex_coord_t(ntx, nty));
			mtl_id_attr.setAttribute(face1, mtl_id);
			faces.push_back(face1);
			face_t* const face2 = new face_t(vertices[a], vertices[d], vertices[b]);
			vert_attrs.set_attribute(face2, 0, options, tex_coord_t(ctx, cty));
			vert_attrs.set_attribute(face2, 1, options, tex_coord_t(ntx, nty));
			vert_attrs.set_attribute(face2, 2, options, tex_coord_t(ntx, cty));
			mtl_id_attr.setAttribute(face2, mtl_id);
			faces.push_back(face2);
		}
	}

	return new mesh_t(faces);
}

void read_weights(const mgl::retained_texture2d_rgba_u8* const map, const u32 x, const u32 y, f64* const w0, f64* const w1, f64* const w2, f64* const w3)
{
	*w0 = (f64)(1.f * map->get_pixel_component(x, y, 0) / MAX_VALUE_TYPE(u8));
	*w1 = (f64)(1.f * map->get_pixel_component(x, y, 1) / MAX_VALUE_TYPE(u8));
	*w2 = (f64)(1.f * map->get_pixel_component(x, y, 2) / MAX_VALUE_TYPE(u8));
	// normalize
	const f64 total = *w0 + *w1 + *w2;
	if (total != 0)
	{
		*w0 /= total;
		*w1 /= total;
		*w2 /= total;
	}
	*w3 = 1 - *w0 - *w1 - *w2;
}
mesh_t* textured_heightmap(carve_vert_attrs& vert_attrs, attr_material_t& mtl_id_attr, const GLuint mtl_id, const heightmap_options& options)
{
	assert(options.width_steps > 1);
	assert(options.depth_steps > 1);

	const bool use_map = options.map;
	const u32 x_steps = use_map ? options.map->get_width() : options.width_steps;
	const u32 z_steps = use_map ? options.map->get_height() : options.depth_steps;
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
			// height is stored in alpha channel
			const f32 y = use_map ? (1.f * options.map->get_pixel_component(ix, (z_steps - 1 - iz), 3) / MAX_VALUE_TYPE(u8)) : 0.f;
			raw_vertices.emplace_back(carve::geom::VECTOR(x, y, z));
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
			const f32 blu = 1.f * ix / (x_steps - 1), blv = 1.f - 1.f * iz / (z_steps - 1);
			const f32 bru = 1.f * (ix + 1) / (x_steps - 1), brv = 1.f - 1.f * iz / (z_steps - 1);
			const f32 tlu = 1.f * ix / (x_steps - 1), tlv = 1.f - 1.f * (iz + 1) / (z_steps - 1);
			const f32 tru = 1.f * (ix + 1) / (x_steps - 1), trv = 1.f - 1.f * (iz + 1) / (z_steps - 1);

			f64 blw0 = options.w0, blw1 = options.w1, blw2 = options.w2, blw3 = options.w3;
			f64 brw0 = options.w0, brw1 = options.w1, brw2 = options.w2, brw3 = options.w3;
			f64 tlw0 = options.w0, tlw1 = options.w1, tlw2 = options.w2, tlw3 = options.w3;
			f64 trw0 = options.w0, trw1 = options.w1, trw2 = options.w2, trw3 = options.w3;
			if (use_map)
			{
				read_weights(options.map, ix + 0, z_steps - 1 - iz, &blw0, &blw1, &blw2, &blw3);
				read_weights(options.map, ix + 1, z_steps - 1 - iz, &brw0, &brw1, &brw2, &brw3);
				read_weights(options.map, ix + 0, z_steps - 2 - iz, &tlw0, &tlw1, &tlw2, &tlw3);
				read_weights(options.map, ix + 1, z_steps - 2 - iz, &trw0, &trw1, &trw2, &trw3);
			}

			// bottom right triangle
			face_t* face = new face_t(vertices[bl], vertices[tr], vertices[br]);
			vert_attrs.set_attribute(face, 0, options, tex_coord_t(blu, blv), blw0, blw1, blw2, blw3);
			vert_attrs.set_attribute(face, 1, options, tex_coord_t(tru, trv), trw0, trw1, trw2, trw3);
			vert_attrs.set_attribute(face, 2, options, tex_coord_t(bru, brv), brw0, brw1, brw2, brw3);
			mtl_id_attr.setAttribute(face, mtl_id);
			faces.push_back(face);
			// top left triangle
			face = new face_t(vertices[bl], vertices[tl], vertices[tr]);
			vert_attrs.set_attribute(face, 0, options, tex_coord_t(blu, blv), blw0, blw1, blw2, blw3);
			vert_attrs.set_attribute(face, 1, options, tex_coord_t(tlu, tlv), tlw0, tlw1, tlw2, tlw3);
			vert_attrs.set_attribute(face, 2, options, tex_coord_t(tru, trv), trw0, trw1, trw2, trw3);
			mtl_id_attr.setAttribute(face, mtl_id);
			faces.push_back(face);
		}
	}

	return new mesh_t(faces);
}
