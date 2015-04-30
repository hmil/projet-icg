#pragma once

#include "HeightmapGenerator.h"
#include "Tile.h"

#define TEXTURE_SIZE 1024
#define TILES_SPAN	5

class Map
{
private:
	HeightmapGenerator generator;
	Grid grid;
	Vec2i active_cell;
	vec2 _cam_pos;
	Tile* active_tiles[TILES_SPAN][TILES_SPAN];


	Tile *generateTile(const int x, const int y) {
		Tile *tile = new Tile(&generator, &grid, x, y);
		tile->generateHeightmap();
		return tile;
	}

public:
	void init(vec2 cam_pos, vec3 fogColor) {
		_cam_pos = cam_pos;
		active_cell = cam_pos.cast<int>() / 4;

		grid.init(fogColor);
		generator.init(1024);

		for (int i = 0; i < TILES_SPAN; ++i) {
			for (int j = 0; j < TILES_SPAN; ++j) {
				active_tiles[i][j] = generateTile(active_cell(0) + i - TILES_SPAN / 2, active_cell(1) + j - TILES_SPAN/2);
            }
		}
	}

	void regenerateHeightmap() {
		for (int i = 0; i < TILES_SPAN; ++i) {
			for (int j = 0; j < TILES_SPAN; ++j) {
				active_tiles[i][j]->generateHeightmap();
			}
		}
	}

	void update(const vec2 cam_pos) {
		_cam_pos(0) = fmod(cam_pos(0), 4);
		_cam_pos(1) = fmod(cam_pos(1), 4);
		Vec2i newCell = cam_pos.cast<int>() / 4;
		
        // Move the map by recycling tiles that are now too far away
		if (newCell(0) != active_cell(0)) {
			std::cout << "x transition : old=" << active_cell(0) << " new=" << newCell(0) << std::endl;
			if (newCell(0) < active_cell(0)) {
				for (int i = 0; i < TILES_SPAN; ++i) {
					Tile* tmp = active_tiles[TILES_SPAN-1][i];
					for (int j = TILES_SPAN-1; j > 0; --j) {
						active_tiles[j][i] = active_tiles[j-1][i];
					}
					active_tiles[0][i] = tmp;
					tmp->setCoords(newCell(0) - TILES_SPAN / 2, newCell(1) - TILES_SPAN / 2 + i);
					tmp->generateHeightmap();
				}
			} else if (newCell(0) > active_cell(0)) {
				for (int i = 0; i < TILES_SPAN; ++i) {
					Tile* tmp = active_tiles[0][i];
					for (int j = 0; j < TILES_SPAN - 1; ++j) {
						active_tiles[j][i] = active_tiles[j+1][i];
					}
					active_tiles[TILES_SPAN - 1][i] = tmp;
					tmp->setCoords(newCell(0) + TILES_SPAN / 2, newCell(1) - TILES_SPAN / 2 + i);
					tmp->generateHeightmap();
				}
			}
		}
		if (newCell(1) != active_cell(1)) {
			std::cout << "y transition : old=" << active_cell(1) << " new=" << newCell(1) << std::endl;
			if (newCell(1) < active_cell(1)) {
				for (int i = 0; i < TILES_SPAN; ++i) {
					Tile *tmp = active_tiles[i][TILES_SPAN-1];
					for (int j = TILES_SPAN - 1; j > 0; --j) {
						active_tiles[i][j] = active_tiles[i][j - 1];
					}
					active_tiles[i][0] = tmp;
					tmp->setCoords(newCell(0) - TILES_SPAN / 2 + i, newCell(1) - TILES_SPAN / 2);
					tmp->generateHeightmap();
				}
			}
			else if (newCell(1) > active_cell(1)) {
				for (int i = 0; i < TILES_SPAN; ++i) {
					Tile *tmp = active_tiles[i][0];
					for (int j = 0; j < TILES_SPAN - 1; ++j) {
						active_tiles[i][j] = active_tiles[i][j + 1];
					}
					active_tiles[i][TILES_SPAN - 1] = tmp;
					tmp->setCoords(newCell(0) - TILES_SPAN / 2 + i, newCell(1) + TILES_SPAN / 2);
					tmp->generateHeightmap();
				}
			}
		}
        // ---

		active_cell = newCell;
	}

	void draw(const mat4 &model, const mat4 &view, const mat4 &projection) {
		//glEnable(GL_CULL_FACE);
        // Matrix to transform each tile
		mat4 tr = mat4::Identity();
		tr(2, 2) = -1; // flip on z axis
		
		// Render the TILES_SPAN*TILES_SPAN grid centered around the camera
		for (int i = 0; i < TILES_SPAN; ++i) {
			for (int j = 0; j < TILES_SPAN; ++j) {

				float pos_x = 4 * (/*active_cell(0)*/ +i - TILES_SPAN / 2) + 2 - _cam_pos(0);
				float pos_y = 4 * (/*active_cell(1)*/ +j - TILES_SPAN / 2) + 2 - _cam_pos(1);

                // translate to appropriate pos
				tr(0, 3) = pos_x;
				tr(2, 3) = pos_y;

				active_tiles[i][j]->draw(model * tr, view, projection, TEXTURE_SIZE);
			}
		}
	}

	void cleanup() {
		for (int i = 0; i < TILES_SPAN; ++i) {
			for (int j = 0; j < TILES_SPAN; ++j) {
				delete active_tiles[i][j];
			}
		}
	}
};
