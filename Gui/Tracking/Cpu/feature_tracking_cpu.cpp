#include <cmath>
#include <cstdlib>
#include <limits>
#include <algorithm>

#include "feature_tracking_cpu.hpp"

struct TempPointData {
	Point location;
	float corner_response;
};

struct sort_by_corner_response {
	bool operator()(TempPointData const &left, TempPointData const &right) {
		return left.corner_response > right.corner_response;
	}
};

FeatureTrackingCpu::FeatureTrackingCpu(const TrackingSettings tracking_settings) :
	FeatureTracking(),
	settings(tracking_settings)
{
	gradient_cols = image_width - 2;
	gradient_rows = image_height - 2;

	blur_gradient_cols = gradient_cols - (filter_range * 2);
	blur_gradient_rows = gradient_rows - (filter_range * 2);

	harris_response_cols = blur_gradient_cols;
	harris_response_rows = blur_gradient_rows;

	normalized_input_image = (float *)malloc(image_width * image_height * sizeof(float));
	tracked_feature_map = (bool *)malloc(image_width * image_height * sizeof(bool));
	memset(tracked_feature_map, false, image_width * image_height * sizeof(bool));
	gradient_x2 = (short *)malloc(gradient_cols * gradient_rows * sizeof(float));
	gradient_y2 = (short *)malloc(gradient_cols * gradient_rows * sizeof(float));
	gradient_xy = (short *)malloc(gradient_cols * gradient_rows * sizeof(float));
	blur_gradient_x2 = (float *)malloc(blur_gradient_cols * blur_gradient_rows * sizeof(float));
	blur_gradient_y2 = (float *)malloc(blur_gradient_cols * blur_gradient_rows * sizeof(float));
	blur_gradient_xy = (float *)malloc(blur_gradient_cols * blur_gradient_rows * sizeof(float));
	harris_response = (float *)malloc(harris_response_cols * harris_response_rows * sizeof(float));
	maxima_suppression = (bool *)malloc(harris_response_cols * harris_response_rows * sizeof(bool));
}

FeatureTrackingCpu::~FeatureTrackingCpu() {
	free(normalized_input_image);
	free(tracked_feature_map);
	free(gradient_x2);
	free(gradient_y2);
	free(gradient_xy);
	free(blur_gradient_x2);
	free(blur_gradient_y2);
	free(blur_gradient_xy);
	free(harris_response);
	free(maxima_suppression);
}

void __inline FeatureTrackingCpu::create_normalized_input_image() {
#pragma loop(hint_parallel(MAX_AP_THREADS))
	for(uint y=0; y<image_height; ++y) {
		for(uint x=0; x<image_width; ++x) {
			const uint idx = idx_1d(x, y, image_width);
			normalized_input_image[idx] = uchar_normalize_table[input_image[idx]];
		}
	}
}

void FeatureTrackingCpu::calc_gradients() {
#pragma loop(hint_parallel(MAX_AP_THREADS))
	for(uint y=1; y<gradient_rows; ++y) {
		for(uint x=1; x<gradient_cols; ++x) {
			const uint gradient_idx = (gradient_cols * (y-1)) + (x-1);

			const short gradient_x = (
				(sobel_x[0] * input_image[idx_1d(x-1, y-1, image_width)]) +
				(sobel_x[1] * input_image[idx_1d(x+0, y-1, image_width)]) +
				(sobel_x[2] * input_image[idx_1d(x+1, y-1, image_width)]) +
				(sobel_x[3] * input_image[idx_1d(x-1, y+0, image_width)]) +
				(sobel_x[4] * input_image[idx_1d(x+0, y+0, image_width)]) +
				(sobel_x[5] * input_image[idx_1d(x+1, y+0, image_width)]) +
				(sobel_x[6] * input_image[idx_1d(x-1, y+1, image_width)]) +
				(sobel_x[7] * input_image[idx_1d(x+0, y+1, image_width)]) +
				(sobel_x[8] * input_image[idx_1d(x+1, y+1, image_width)])
			);

			const short gradient_y = (
				(sobel_y[0] * input_image[idx_1d(x-1, y-1, image_width)]) +
				(sobel_y[1] * input_image[idx_1d(x+0, y-1, image_width)]) +
				(sobel_y[2] * input_image[idx_1d(x+1, y-1, image_width)]) +
				(sobel_y[3] * input_image[idx_1d(x-1, y+0, image_width)]) +
				(sobel_y[4] * input_image[idx_1d(x+0, y+0, image_width)]) +
				(sobel_y[5] * input_image[idx_1d(x+1, y+0, image_width)]) +
				(sobel_y[6] * input_image[idx_1d(x-1, y+1, image_width)]) +
				(sobel_y[7] * input_image[idx_1d(x+0, y+1, image_width)]) +
				(sobel_y[8] * input_image[idx_1d(x+1, y+1, image_width)])
			);

			gradient_x2[gradient_idx] = gradient_x * gradient_x;
			gradient_y2[gradient_idx] = gradient_y * gradient_y;
			gradient_xy[gradient_idx] = gradient_x  * gradient_y;
		}
	}
}

void FeatureTrackingCpu::blur_gradient(short *gradient_img, float *blur_gradient_img) {
#pragma loop(hint_parallel(MAX_AP_THREADS))
	for(uint y=filter_range; y<gradient_rows-filter_range; ++y) {
		for(uint x=filter_range; x<gradient_cols-filter_range; ++x) {
			float total = 0.0;

			for(uint y2=y-filter_range, gauss_y=0; y2<y+filter_range; ++y2, ++gauss_y) {
				for(uint x2=x-filter_range, gauss_x=0; x2<x+filter_range; ++x2, ++gauss_x) {
					total += gaussian_matrix[idx_1d(gauss_x, gauss_y, filter_width)] * gradient_img[idx_1d(x2, y2, gradient_cols)];
				}
			}

			blur_gradient_img[idx_1d(x-filter_range, y-filter_range, blur_gradient_cols)] = total;
		}
	}
}

void FeatureTrackingCpu::calc_harris_response() {
#pragma loop(hint_parallel(MAX_AP_THREADS))
	for(uint y=0; y<blur_gradient_rows; ++y) {
		for(uint x=0; x<blur_gradient_cols; ++x) {
			const uint idx = idx_1d(x, y, blur_gradient_cols);

			const float gx2 = blur_gradient_x2[idx];
			const float gy2 = blur_gradient_y2[idx];
			const float gxy = blur_gradient_xy[idx];

			const float det = (gx2 * gy2) - (gxy * gxy);
			const float trace = gx2 + gy2;

			harris_response[idx] = det - (settings.sensitivity * (trace * trace));
		}
	}
}

void __inline FeatureTrackingCpu::blur_gradients() {
	blur_gradient(gradient_x2, blur_gradient_x2);
	blur_gradient(gradient_y2, blur_gradient_y2);
	blur_gradient(gradient_xy, blur_gradient_xy);
}

void FeatureTrackingCpu::get_maxima_points() {
	memset(maxima_suppression, 1, harris_response_cols * harris_response_rows * sizeof(bool));

	std::vector<TempPointData> points;
#pragma loop(hint_parallel(MAX_AP_THREADS))
	for(uint y=0; y<harris_response_rows; ++y) {
		for(uint x=0; x<harris_response_cols; ++x) {
			TempPointData d;
			d.corner_response = harris_response[idx_1d(x, y, harris_response_cols)];
			d.location.x = x;
			d.location.y = y;

			if(d.corner_response > settings.harris_response_threshhold) {
				points.push_back(d);
			}
		}
	}

	sort(points.begin(), points.end(), sort_by_corner_response());

	harris_points.clear();

	for(size_t i=0; harris_points.size()<settings.max_tracked_features && i<points.size(); ++i) {
		if(maxima_suppression[idx_1d(points[i].location.x, points[i].location.y, harris_response_cols)] == true) {
			for(char y=-maxima_suppression_range; y<=maxima_suppression_range; ++y) {
				for(char x=-maxima_suppression_range; x<=maxima_suppression_range; ++x) {
					int sx = points[i].location.x + x;
					int sy = points[i].location.y + y;
					sx = sx >= harris_response_cols ? harris_response_cols-1 : sx < 0 ? 0 : sx;
					sy = sy >= harris_response_rows ? harris_response_rows-1 : sy < 0 ? 0 : sy;

					const uint idx = idx_1d(sx, sy, harris_response_cols);
					maxima_suppression[idx] = false;
				}
			}

			HarrisPoint harris_point;
			harris_point.locations[0].x = points[i].location.x + 1 + filter_range;
			harris_point.locations[0].y = points[i].location.y + 1 + filter_range;

			for(char window_offset_y=-3, template_y=0; window_offset_y<=3; ++window_offset_y, ++template_y) {
				for(char window_offset_x=-3, template_x=0; window_offset_x<=3; ++window_offset_x, ++template_x) {
					int window_x = harris_point.locations[0].x + window_offset_x;
					int window_y = harris_point.locations[0].y + window_offset_y;
					window_x = window_x >= image_width ? image_width-1 : window_x < 0 ? 0 : window_x;
					window_y = window_y >= image_height ? image_height-1 : window_y < 0 ? 0 : window_y;
					harris_point.signature[(template_y * 7) + template_x] = normalized_input_image[idx_1d(window_x, window_y, image_width)];
				}
			}

			harris_points.push_back(harris_point);
		}
	}
}

float __inline FeatureTrackingCpu::get_template_average(float *signature) {
	float template_average = 0.0f;
	for(uchar i=0; i<49; ++i) {
		template_average += signature[i];
	}
	return template_average /= 49.0f;
}

float FeatureTrackingCpu::get_window_average(uint x, uint y) {
	float window_average = 0.0f;
	for(char window_offset_y=-3; window_offset_y<=3; ++window_offset_y) {
		for(char window_offset_x=-3; window_offset_x<=3; ++window_offset_x) {
			int window_x = x + window_offset_x;
			int window_y = y + window_offset_y;
			window_x = window_x >= image_width ? image_width-1 : window_x < 0 ? 0 : window_y;
			window_y = window_y >= image_height ? image_height-1 : window_y < 0 ? 0 : window_y;
			window_average += normalized_input_image[idx_1d(window_x, window_y, image_width)];
		}
	}
	return window_average /= 49.0f;
}

bool FeatureTrackingCpu::track_point(Point old_location, float *signature, Point &new_location) {
	/* Calculate average for the point's 7x7 template window */
	const float template_average = get_template_average(signature);

	/* Track maximum correlation value and its location */
	float max_correlation_value = std::numeric_limits<float>::min();
	Point max_correlation_point;
	max_correlation_point.x = 0;
	max_correlation_point.y = 0;

	/* Evaluate correlation value of each pixel in an area around the current tracked feature */
	for(char search_area_offset_y=-3; search_area_offset_y<=3; ++search_area_offset_y) {
		for(char search_area_offset_x=-3; search_area_offset_x<=3; ++search_area_offset_x) {
			/* Add offset to current tracked feature to get X and Y coordinates of point
			in the search area currently being evaluated for correlation */
			const int search_area_x = old_location.x + search_area_offset_x;
			const int search_area_y = old_location.y + search_area_offset_y;

			/* Return if this point in the window is outside the image */
			if(search_area_x>=image_width || search_area_x<0 || search_area_y>=image_height || search_area_y<0) {
				break;
			}

			/* Sum up intermediary values in a window around the current search area pixel
			for calculating the correlation value for that pixel */
			float ixy = 0.0f;
			float ix2 = 0.0f;
			float iy2 = 0.0f;
			for(char window_offset_y=-3, template_y=0; window_offset_y<=3; ++window_offset_y, ++template_y) {
				for(char window_offset_x=-3, template_x=0; window_offset_x<=3; ++window_offset_x, ++template_x) {
					int window_x = search_area_x + window_offset_x;
					int window_y = search_area_y + window_offset_y;
					window_x = window_x >= image_width ? image_width-1 : window_x < 0 ? 0 : window_x;
					window_y = window_y >= image_height ? image_height-1 : window_y < 0 ? 0 : window_y;

					float window_average = get_window_average(window_x, window_y);

					float pixel_value = normalized_input_image[idx_1d(window_x, window_y, image_width)];
					float template_value = signature[(template_y * 7) + template_x];

					float ix = pixel_value - window_average;
					float iy = template_value - template_average;

					ixy += ix * iy;
					ix2 += ix * ix;
					iy2 += iy * iy;
				}
			}

			/* Calculate correlation value for the current search area pixel */
			float correlation = ixy / sqrt(ix2 * iy2);

			/* If this correlation value is the new highest */
			/* Update the current highest correlation value and location */
			if(correlation > max_correlation_value) {
				max_correlation_value = correlation;
				max_correlation_point.x = search_area_x;
				max_correlation_point.y = search_area_y;
			}
		}
	}

	new_location = max_correlation_point;
	return max_correlation_value >= settings.correlation_threshhold;
}

void FeatureTrackingCpu::update_tracked_features() {
	if(image_count == 0) {
		/* If this is the first image, just use the harris corners detected */
		tracked_features = harris_points;
		for(int i=tracked_features.size()-1; i>=0; --i) {
			const uint idx = tracked_features[i].location_idx;
			const uint x = tracked_features[i].locations[idx].x;
			const uint y = tracked_features[i].locations[idx].y;
			tracked_feature_map[idx_1d(x, y, image_width)] = true;
		}
		return;
	}

	/* For each previously tracked feature */
	for(uint i=0; i<tracked_features.size(); ++i) {
		HarrisPoint *feature = &tracked_features[i];

		Point new_location;

		Point max_correlation_point;
		bool over_threshhold = track_point(feature->locations[feature->location_idx], feature->signature, max_correlation_point);
		bool track_success;

		if((tracked_features[i].track_frames + 1) % (settings.template_update_frames * 2) == 0) {
			Point max_correlation_point_new_template;
			bool over_threshhold_new_template = track_point(feature->locations[feature->location_idx], feature->new_signature, max_correlation_point_new_template);
			if(over_threshhold_new_template && distance(max_correlation_point, max_correlation_point_new_template) < settings.template_update_distance_threshhold) {
				memcpy(feature->new_signature, feature->signature, 49 * sizeof(float));
				new_location = max_correlation_point_new_template;
				track_success = true;
			} else {
				track_success = false;
			}
		} else {
			new_location = max_correlation_point;
			track_success = over_threshhold;
		}

		if(track_success) {
			++tracked_features[i].track_frames;

			/* Remove old tracked point from map */
			const uint x = tracked_features[i].locations[tracked_features[i].location_idx].x;
			const uint y = tracked_features[i].locations[tracked_features[i].location_idx].y;
			tracked_feature_map[idx_1d(x, y, image_width)] = false;

			/* Add new tracked point to map */
			tracked_feature_map[idx_1d(new_location.x, new_location.y, image_width)] = true;

			/* Add the new location to the tracked feature's location history */
			tracked_features[i].location_idx = (tracked_features[i].location_idx + 1) % MAX_TRACKED_POINT_LOCATIONS;
			tracked_features[i].locations[tracked_features[i].location_idx] = new_location;

			/* If we have tracked this point for enough frames to trigger template updating */
			if(tracked_features[i].track_frames % settings.template_update_frames == 0 && (tracked_features[i].track_frames + 1) % (settings.template_update_frames * 2) != 0) {
				/* Update the tracked feature's 7x7 template to that of its current location in the image */
				for(char window_offset_y=-3, template_y=0; window_offset_y<=3; ++window_offset_y, ++template_y) {
					for(char window_offset_x=-3, template_x=0; window_offset_x<=3; ++window_offset_x, ++template_x) {
						int window_x = new_location.x + window_offset_x;
						int window_y = new_location.y + window_offset_y;
						window_x = window_x >= image_width ? image_width-1 : window_x < 0 ? 0 : window_x;
						window_y = window_y >= image_height ? image_height-1 : window_y < 0 ? 0 : window_y;

						feature->new_signature[(template_y * 7) + template_x] = normalized_input_image[idx_1d(window_x, window_y, image_width)];
					}
				}
			}
		} else {
			/* Correlation value was not above threshhold feature has been lost, 
			remove it from the vector and tracked feature map */
			const uint x = tracked_features[i].locations[tracked_features[i].location_idx].x;
			const uint y = tracked_features[i].locations[tracked_features[i].location_idx].y;
			tracked_feature_map[idx_1d(x, y, image_width)] = false;

			tracked_features.erase(tracked_features.begin() + i);
		}
	}

	/* Add harris points to the tracked features list if they
	are far enough away from existing tracked features */
	for(int i=harris_points.size()-1; i>=0; --i) {
		const uint x = harris_points[i].locations[0].x;
		const uint y = harris_points[i].locations[0].y;
		for(char window_offset_y=-3; window_offset_y<=3; ++window_offset_y) {
			for(char window_offset_x=-3; window_offset_x<=3; ++window_offset_x) {
				int window_x = x + window_offset_x;
				int window_y = y + window_offset_y;
				window_x = window_x >= image_width ? image_width-1 : window_x < 0 ? 0 : window_x;
				window_y = window_y >= image_height ? image_height-1 : window_y < 0 ? 0 : window_y;

				if(tracked_feature_map[idx_1d(window_x, window_y, image_width)] == true) {
					goto NEXT_POINT;
				}
			}
		}
		tracked_features.push_back(harris_points[i]);
		tracked_feature_map[idx_1d(x, y, image_width)] = true;
		if(tracked_features.size() >= settings.max_tracked_features) {
			break;
		}
NEXT_POINT:;
	}
}

std::vector<HarrisPoint> FeatureTrackingCpu::feature_points(uchar *input) {
	input_image = input;

	create_normalized_input_image();
	calc_gradients();
	blur_gradients();
	calc_harris_response();
	get_maxima_points();
	update_tracked_features();

	++image_count;

	return tracked_features;
}
