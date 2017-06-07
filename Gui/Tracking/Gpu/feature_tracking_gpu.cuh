#pragma once
#ifndef FEATURE_TRACKING_GPU_CUH
#define FEATURE_TRACKING_GPU_CUH

#include <vector>

#include "cuda_runtime.h"

#include "utils/utils.hpp"
#include "Tracking/feature_tracking.hpp"

__device__ struct d_Point {
	uint x, y;
};

__device__ struct d_HarrisPoint {
	d_Point locations[MAX_TRACKED_POINT_LOCATIONS];
	uint location_idx = 0;
	float signature[49];
	float new_signature[49];
	uint track_frames = 0;
	bool tracked = true;
};

__device__ struct d_Correlation {
	d_Point location;
	float correlation;
};

class FeatureTrackingGpu : public FeatureTracking {
public:
	FeatureTrackingGpu(int device, const TrackingSettings &tracking_settings);
	~FeatureTrackingGpu();
	std::vector<HarrisPoint> feature_points(uchar *input) override;

private:
	const TrackingSettings &settings;

	int device;
	int block_side_len;
	dim3 block_size;
	dim3 input_image_grid_size;
	dim3 gradient_grid_size;
	dim3 blur_gradient_grid_size;
	dim3 harris_response_grid_size;

	size_t input_image_size;

	uint gradient_cols;
	uint gradient_rows;
	uint blur_gradient_cols;
	uint blur_gradient_rows;
	uint harris_response_cols;
	uint harris_response_rows;

	uchar *h_input_image;
	HarrisPoint *h_tracked_features;
	bool *h_tracked_feature_map;

	char *d_sobel_x;
	char *d_sobel_y;
	float *d_gaussian_matrix;
	float *d_uchar_normalize_table;
	uchar *d_input_image;
	float *d_normalized_input_image;
	short *d_gradient_x2;
	short *d_gradient_y2;
	short *d_gradient_xy;
	float *d_blur_gradient_x2;
	float *d_blur_gradient_y2;
	float *d_blur_gradient_xy;
	float *d_harris_response;
	bool *d_maxima_suppression;
	uint *d_points_after_suppression;
	bool *d_tracked_feature_map;
	d_HarrisPoint *d_tracked_features;
	d_Correlation *d_correlation_map;
	d_Correlation *d_correlation_map_new_template;

	std::vector<HarrisPoint> harris_points;
	std::vector<HarrisPoint> tracked_features;

	uint image_count = 0;

	void create_normalised_input_image();
	void calc_gradients();
	void blur_gradients();
	void calc_harris_response();
	void get_maxima_points();
	void update_tracked_features();
};

#endif CORNER_DETECTION_CUH
