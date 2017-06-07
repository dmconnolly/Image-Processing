#pragma once
#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>

#include "Utils/types.hpp"

#define MAX_AP_THREADS 0

struct Colour {
	uchar r, g, b;
	Colour() {
		/* Empty */
	}
	Colour(uchar r, uchar g, uchar b) {
		this->r = r;
		this->g = g;
		this->b = b;
	}
};

static __forceinline uint idx_1d(uint x, uint y, uint width) {
	return (width * y) + x;
}

static void gray_arr_to_rgb_mat(uchar *input, uchar *output, uint cols, uint rows) {
#pragma loop(hint_parallel(MAX_AP_THREADS))
	for(uint i=0; i<cols*rows; ++i) {
		output[(i*3)+0] = input[i];
		output[(i*3)+1] = input[i];
		output[(i*3)+2] = input[i];
	}
}

static void mark_point(
	uchar *image,
	uint cols, uint rows,
	uint point_x, uint point_y,
	uchar radius, Colour colour)
{
	for(char y=-radius; y<radius; ++y) {
		for(char x=-radius; x<radius; ++x) {
			int x2 = point_x + x;
			int y2 = point_y + y;

			if(x2 < 0 || x2 >= cols || y2 < 0 || y2 >= rows) {
				break;
			}

			uint idx = idx_1d(x2, y2, cols) * 3;
			image[idx + 0] = colour.b;
			image[idx + 1] = colour.g;
			image[idx + 2] = colour.r;
		}
	}
}

#endif /* UTILS_HPP */
