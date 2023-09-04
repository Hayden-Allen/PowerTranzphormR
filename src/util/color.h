#pragma once
#include "pch.h"

struct color_t
{
	f32 r, g, b, a;
	color_t() :
		r(0.f), g(0.f), b(0.f), a(0.f)
	{}
	color_t(const f32 _r, const f32 _g, const f32 _b, const f32 _a) :
		r(_r), g(_g), b(_b), a(_a)
	{}
};
static color_t operator*(const f64 s, const color_t& c)
{
	const f32 fs = (f32)s;
	return color_t(c.r * fs, c.g * fs, c.b * fs, c.a * fs);
}
static color_t& operator+=(color_t& t1, const color_t& t2)
{
	t1.r += t2.r;
	t1.g += t2.g;
	t1.b += t2.b;
	t1.a += t2.a;
	return t1;
}