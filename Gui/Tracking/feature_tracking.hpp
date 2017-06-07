#pragma once
#ifndef FEATURE_TRACKING_HPP
#define FEATURE_TRACKING_HPP

#include <vector>

#include "Utils/utils.hpp"
#include "Utils/types.hpp"

#define MAX_TRACKED_POINT_LOCATIONS 200

struct Point {
	uint x, y;
	Point() {
		/* Empty */
	}
	Point(uint x, uint y) {
		this->x = x;
		this->y = y;
	}
};

struct PointData {
	Point location;
	float corner_response;
	float signature[49];
};

struct HarrisPoint {
	Point locations[MAX_TRACKED_POINT_LOCATIONS];
	uint location_idx = 0;
	float signature[49];
	float new_signature[49];
	uint track_frames = 0;
	bool tracked = true;
};

struct TrackingSettings {
	uint max_frames;
	float sensitivity;
	uint max_tracked_features;
	float harris_response_threshhold;
	float correlation_threshhold;
	uint template_update_frames;
	float template_update_distance_threshhold;
};

static void mark_feature_points(
	uchar *image,
	std::vector<HarrisPoint> points,
	uint cols, uint rows,
	uchar radius, Colour colour)
{
	for(size_t j=0; j<points.size(); ++j) {
		Point *locations = points[j].locations;
		if(points[j].track_frames > 1) {
			const uint min = points[j].track_frames < MAX_TRACKED_POINT_LOCATIONS ? points[j].track_frames : MAX_TRACKED_POINT_LOCATIONS;
			for(uint k=0; k<min; ++k) {
				mark_point(image, cols, rows, locations[k].x, locations[k].y, 1, colour);
			}
		}
	}
}

static __inline float distance(Point p1, Point p2) {
	const float diff_x = (long)p1.x - (long)p2.x;
	const float diff_y = (long)p1.y - (long)p2.y;
	return std::sqrt((diff_x * diff_x) + (diff_y * diff_y));
}

class FeatureTracking {
protected:
	const static uint image_width = 1024;
	const static uint image_height = 768;
	const static float uchar_normalize_table[256];
	const static char sobel_x[9];
	const static char sobel_y[9];
	const static float gaussian_matrix[49];
	const static char filter_width = 7;
	const static char filter_range = 3;
	const static char maxima_suppression_width = 7;
	const static char maxima_suppression_range = 3;
public:
	virtual std::vector<HarrisPoint> feature_points(uchar *input) = 0;
	virtual ~FeatureTracking() {}
};

#endif /* FEATURE_TRACKING_HPP */
