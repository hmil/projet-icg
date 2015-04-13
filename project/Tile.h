#pragma once

#include "HeightmapGenerator.h"
#include "FrameBuffer.h"
#include "Grid.h"

#define TEXTURE_SIZE 1024


static float H = 0.85f;
static float lacunarity = 2;
static int octaves = 8;

class Tile {
private:
	FrameBuffer _fb;
	Grid _grid;
	HeightmapGenerator *_gen;
	vec2 _offset;

public:

	Tile(HeightmapGenerator *gen, int x, int y) :
		_fb(TEXTURE_SIZE, TEXTURE_SIZE)
	{
		_gen = gen; 
		GLuint tex_coord = _fb.init(true /* use interpolation */);
		_grid.init(tex_coord);
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
		_grid.draw(model, view, projection, resolution);
	}


};