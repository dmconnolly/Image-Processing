#pragma once
#ifndef FEATURE_TRACKING_CPU_HPP
#define FEATURE_TRACKING_CPU_HPP

#include <vector>

#include "Utils/utils.hpp"
#include "Tracking/feature_tracking.hpp"

class FeatureTrackingCpu : public FeatureTracking {
public:
	FeatureTrackingCpu(const TrackingSettings tracking_settings);
	~FeatureTrackingCpu();
	std::vector<HarrisPoint> feature_points(uchar *input) override;

private:
	const TrackingSettings &settings;

	uint gradient_cols;
	uint gradient_rows;
	uint blur_gradient_cols;
	uint blur_gradient_rows;
	uint harris_response_cols;
	uint harris_response_rows;

	uchar *input_image;
	float *normalized_input_image;
	short *gradient_x2;
	short *gradient_y2;
	short *gradient_xy;
	float *blur_gradient_x2;
	float *blur_gradient_y2;
	float *blur_gradient_xy;
	float *harris_response;
	bool *maxima_suppression;
	bool *tracked_feature_map;

	std::vector<HarrisPoint> harris_points;
	std::vector<HarrisPoint> tracked_features;

	int image_count = 0;

	void __inline create_normalized_input_image();
	void calc_gradients();
	void blur_gradient(short *gradient_img, float *blur_gradient_img);
	void __inline blur_gradients();
	void calc_harris_response();
	void get_maxima_points();
	void update_tracked_features();

	float __inline get_template_average(float *signature);
	float __inline get_window_average(uint x, uint y);
	bool track_point(Point old_location, float *signature, Point &new_location);
};

#endif /* FEATURE_TRACKING_CPU_HPP */
