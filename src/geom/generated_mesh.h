#pragma once
#include "pch.h"
#include "carve.h"

class generated_mesh
{
public:
	mesh_t* mesh;
public:
	generated_mesh(mesh_t* const m) :
		mesh(m)
	{}
	MGL_DCM(generated_mesh);
	virtual ~generated_mesh() {}
};

class generated_cuboid : public generated_mesh
{
public:
	generated_cuboid(mesh_t* const m, const cuboid_options& opts) :
		generated_mesh(m),
		m_options(opts)
	{}
private:
	cuboid_options m_options;
};

class generated_ellipsoid : public generated_mesh
{
public:
	generated_ellipsoid(mesh_t* const m, const ellipsoid_options& opts) :
		generated_mesh(m),
		m_options(opts)
	{}
private:
	ellipsoid_options m_options;
};

class generated_cylinder : public generated_mesh
{
public:
	generated_cylinder(mesh_t* const m, const cylinder_options& opts) :
		generated_mesh(m),
		m_options(opts)
	{}
private:
	cylinder_options m_options;
};

class generated_cone : public generated_mesh
{
public:
	generated_cone(mesh_t* const m, const cone_options& opts) :
		generated_mesh(m),
		m_options(opts)
	{}
private:
	cone_options m_options;
};

class generated_torus : public generated_mesh
{
public:
	generated_torus(mesh_t* const m, const torus_options& opts) :
		generated_mesh(m),
		m_options(opts)
	{}
private:
	torus_options m_options;
};

class generated_heightmap : public generated_mesh
{
public:
	generated_heightmap(mesh_t* const m, const heightmap_options& opts) :
		generated_mesh(m),
		m_options(opts)
	{}
private:
	heightmap_options m_options;
};
