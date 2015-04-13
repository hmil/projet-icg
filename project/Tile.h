#pragma once

#include "HeightmapGenerator.h"
#include "FrameBuffer.h"
#include "Grid.h"

#define TEXTURE_SIZE 1024


float H = 0.85f;
float lacunarity = 2;
int octaves = 8;

class Tile {
private:
	FrameBuffer _fb;
	Grid *_grid;
	HeightmapGenerator *_gen;
	vec2 _offset;
	GLuint fb_tex;

public:

	Tile(HeightmapGenerator *gen, Grid *grid, int x, int y) :
		_fb(TEXTURE_SIZE, TEXTURE_SIZE)
	{
		_gen = gen;
		_grid = grid;
		fb_tex = _fb.init(true /* use interpolation */);
		_offset(0) = x;
		_offset(1) = y;
	}

	void setCoords(int x, int y) {
		_offset(0) = x;
		_offset(1) = y;
	}

	void generateHeightmap() {
		_fb.bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		_gen->drawHeights(H, lacunarity, octaves, _offset);
		_fb.unbind();
	}

	void draw(const mat4& model, const mat4& view, const mat4& projection, const int resolution){
		_grid->draw(model, view, projection, resolution, fb_tex);
	}


};