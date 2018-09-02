#pragma once

struct Glyph
{
	char id;
	float32 u, v;
	float32 x, y;
	float32 w, h;
	float32 x_advance;
};

struct Kerning
{
	char first;
	char second;
	float32 amount;
};

