#include <cstdlib>
#include <algorithm>

#include "Utils/utils.hpp"

#include "feature_tracking_gpu.cuh"
#include "helper_cuda.h"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

__device__ struct d_PointData {
	d_Point location;
	float corner_response;
	float signature[49];
};

__device__ __forceinline__ uint d_idx_1d(uint x, uint y, uint width) {
	return (width * y) + x;
}

__device__ __forceinline__ float d_get_template_average(float *signature) {
	float template_average = 0.0f;
	for(uchar i=0; i<49; ++i) {
		template_average += signature[i];
	}
	return template_average /= 49.0f;
}

__device__ float d_get_window_average(
	uint x, uint y,
	uint num_cols, uint num_rows,
	float * __restrict normalized_input_image)
{
	float window_average = 0.0f;
	for(int window_offset_y=-3; window_offset_y<=3; ++window_offset_y) {
		for(int window_offset_x=-3; window_offset_x<=3; ++window_offset_x) {
			int window_x = x + window_offset_x;
			int window_y = y + window_offset_y;
			window_x = window_x >= num_cols ? num_cols-1 : window_x < 0 ? 0 : window_y;
			window_y = window_y >= num_rows ? num_rows-1 : window_y < 0 ? 0 : window_y;
			window_average += normalized_input_image[d_idx_1d(window_x, window_y, num_cols)];
		}
	}
	return window_average /= 49.0f;
}

__global__ void _create_normalised_input_image(
	uchar * __restrict input_image,
	float * __restrict uchar_normalize_table,
	float * __restrict normalized_input_image,
	uint num_cols,
	uint num_rows)
{
	const uint2 thread_2D_pos = make_uint2(
		blockIdx.x * blockDim.x + threadIdx.x,
		blockIdx.y * blockDim.y + threadIdx.y
	);

	const uint thread_1D_pos = d_idx_1d(thread_2D_pos.x, thread_2D_pos.y, num_cols);

	if(thread_2D_pos.x >= num_cols || thread_2D_pos.y >= num_rows) {
		return;
	}

	normalized_input_image[thread_1D_pos] = uchar_normalize_table[input_image[thread_1D_pos]];
}

__global__ void _calc_gradients(
	uchar * __restrict input_image,
	short * __restrict gradient_x2,
	short * __restrict gradient_y2,
	short * __restrict gradient_xy,
	char * __restrict sobel_x,
	char * __restrict sobel_y,
	uint num_cols,
	uint gradient_cols,
	uint gradient_rows)
{
	const uint2 thread_2D_pos = make_uint2(
		blockIdx.x * blockDim.x + threadIdx.x,
		blockIdx.y * blockDim.y + threadIdx.y
	);

	const uint thread_1D_pos = d_idx_1d(thread_2D_pos.x, thread_2D_pos.y, gradient_cols);

	if(thread_2D_pos.x >= gradient_cols || thread_2D_pos.y >= gradient_rows) {
		return;
	}

	const uint x = thread_2D_pos.x + 1;
	const uint y = thread_2D_pos.y + 1;

	const short gradient_x = (
		(sobel_x[0] * input_image[d_idx_1d(x-1, y-1, num_cols)]) +
		(sobel_x[1] * input_image[d_idx_1d(x+0, y-1, num_cols)]) +
		(sobel_x[2] * input_image[d_idx_1d(x+1, y-1, num_cols)]) +
		(sobel_x[3] * input_image[d_idx_1d(x-1, y+0, num_cols)]) +
		(sobel_x[4] * input_image[d_idx_1d(x+0, y+0, num_cols)]) +
		(sobel_x[5] * input_image[d_idx_1d(x+1, y+0, num_cols)]) +
		(sobel_x[6] * input_image[d_idx_1d(x-1, y+1, num_cols)]) +
		(sobel_x[7] * input_image[d_idx_1d(x+0, y+1, num_cols)]) +
		(sobel_x[8] * input_image[d_idx_1d(x+1, y+1, num_cols)])
	);

	const short gradient_y = (
		(sobel_y[0] * input_image[d_idx_1d(x-1, y-1, num_cols)]) +
		(sobel_y[1] * input_image[d_idx_1d(x+0, y-1, num_cols)]) +
		(sobel_y[2] * input_image[d_idx_1d(x+1, y-1, num_cols)]) +
		(sobel_y[3] * input_image[d_idx_1d(x-1, y+0, num_cols)]) +
		(sobel_y[4] * input_image[d_idx_1d(x+0, y+0, num_cols)]) +
		(sobel_y[5] * input_image[d_idx_1d(x+1, y+0, num_cols)]) +
		(sobel_y[6] * input_image[d_idx_1d(x-1, y+1, num_cols)]) +
		(sobel_y[7] * input_image[d_idx_1d(x+0, y+1, num_cols)]) +
		(sobel_y[8] * input_image[d_idx_1d(x+1, y+1, num_cols)])
	);

	gradient_x2[thread_1D_pos] = gradient_x * gradient_x;
	gradient_y2[thread_1D_pos] = gradient_y * gradient_y;
	gradient_xy[thread_1D_pos] = gradient_x * gradient_y;
}

__global__ void _blur_gradients(
	short * __restrict gradient_image,
	float * __restrict blur_gradient_image,
	float *__restrict gaussian_matrix,
	char filter_range,
	uint gradient_cols,
	uint blur_gradient_cols,
	uint blur_gradient_rows)
{
	const uint2 thread_2D_pos = make_uint2(
		blockIdx.x * blockDim.x + threadIdx.x,
		blockIdx.y * blockDim.y + threadIdx.y
	);

	const uint thread_1D_pos = d_idx_1d(thread_2D_pos.x, thread_2D_pos.y, blur_gradient_cols);

	if(thread_2D_pos.x >= blur_gradient_cols || thread_2D_pos.y >= blur_gradient_rows) {
		return;
	}

	float total = 0.0f;
	for(uint y=thread_2D_pos.y-filter_range, gauss_idx=0; y<thread_2D_pos.y+filter_range; ++y, ++gauss_idx) {
		for(uint x=thread_2D_pos.x-filter_range; x<thread_2D_pos.x+filter_range; ++x, ++gauss_idx) {
			total += gaussian_matrix[gauss_idx] * gradient_image[d_idx_1d(x+filter_range, y+filter_range, gradient_cols)];
		}
	}
	blur_gradient_image[d_idx_1d(thread_2D_pos.x, thread_2D_pos.y, blur_gradient_cols)] = total;
}

__global__ void _calc_harris_response(
	float * __restrict blur_gradient_x2,
	float * __restrict blur_gradient_y2,
	float * __restrict blur_gradient_xy,
	float * __restrict harris_response,
	float sensitivity,
	uint blur_gradient_cols,
	uint blur_gradient_rows)
{
	const uint2 thread_2D_pos = make_uint2(
		blockIdx.x * blockDim.x + threadIdx.x,
		blockIdx.y * blockDim.y + threadIdx.y
	);

	const uint thread_1D_pos = d_idx_1d(thread_2D_pos.x, thread_2D_pos.y, blur_gradient_cols);

	if(thread_2D_pos.x >= blur_gradient_cols || thread_2D_pos.y >= blur_gradient_rows) {
		return;
	}

	float gx2 = blur_gradient_x2[thread_1D_pos];
	float gy2 = blur_gradient_y2[thread_1D_pos];
	float gxy = blur_gradient_xy[thread_1D_pos];

	float det = (gx2 * gy2) - (gxy * gxy);
	float trace = gx2 + gy2;

	harris_response[thread_1D_pos] = det - (sensitivity * (trace * trace));
}

__global__ void _non_maximum_suppression(
	float * __restrict harris_response,
	bool * __restrict maxima_suppression,
	int maxima_suppression_range,
	float threshhold,
	uint harris_response_cols,
	uint harris_response_rows,
	uint * __restrict points_after_suppression)
{
	const uint2 thread_2D_pos = make_uint2(
		blockIdx.x * blockDim.x + threadIdx.x,
		blockIdx.y * blockDim.y + threadIdx.y
	);

	const uint thread_1D_pos = d_idx_1d(thread_2D_pos.x, thread_2D_pos.y, harris_response_cols);

	if(thread_2D_pos.x >= harris_response_cols || thread_2D_pos.y >= harris_response_rows) {
		return;
	}

	float current_response = harris_response[thread_1D_pos];

	for(char window_offset_y=-maxima_suppression_range; window_offset_y<=maxima_suppression_range; ++window_offset_y) {
		for(char window_offset_x=-maxima_suppression_range; window_offset_x<=maxima_suppression_range; ++window_offset_x) {
			int window_x = thread_2D_pos.x + window_offset_x;
			int window_y = thread_2D_pos.y + window_offset_y;
			if(window_x < 0 || window_x >= harris_response_cols || window_y < 0 || window_y >= harris_response_rows) {
				break;
			}
			if(window_offset_x == 0 && window_offset_y == 0) {
				if(current_response < threshhold) {
					maxima_suppression[thread_1D_pos] = false;
				}
				break;
			}

			uint window_idx = d_idx_1d(window_x, window_y, harris_response_cols);
			float window_point_response = harris_response[window_idx];

			if(current_response > window_point_response) {
				maxima_suppression[window_idx] = false;
			}
		}
	}

	__syncthreads();

	if(maxima_suppression[thread_1D_pos] == true) {
		atomicAdd(points_after_suppression, 1);
	}
}

__global__ void _fill_points(
	float * __restrict harris_response,
	bool * __restrict maxima_suppression,
	d_PointData * __restrict points,
	char filter_range,
	uint harris_response_cols,
	uint harris_response_rows,
	uint num_cols,
	uint num_rows,
	float * __restrict normalized_input_image,
	uint * __restrict points_after_suppression)
{
	const uint2 thread_2D_pos = make_uint2(
		blockIdx.x * blockDim.x + threadIdx.x,
		blockIdx.y * blockDim.y + threadIdx.y
	);

	const uint thread_1D_pos = d_idx_1d(thread_2D_pos.x, thread_2D_pos.y, harris_response_cols);

	if(thread_2D_pos.x >= harris_response_cols || thread_2D_pos.y >= harris_response_rows) {
		return;
	}

	if(maxima_suppression[thread_1D_pos]) {
		const uint write_idx = atomicAdd(points_after_suppression, 1);

		d_Point point;
		point.x = thread_2D_pos.x + 1 + filter_range;
		point.y = thread_2D_pos.y + 1 + filter_range;

		d_PointData point_data;
		point_data.corner_response = harris_response[thread_1D_pos];
		point_data.location = point;

		for(char window_offset_y=-3, template_y=0; window_offset_y<=3; ++window_offset_y, ++template_y) {
			for(char window_offset_x=-3, template_x=0; window_offset_x<=3; ++window_offset_x, ++template_x) {
				int window_x = point.x + window_offset_x;
				int window_y = point.y + window_offset_y;
				window_x = window_x >= num_cols ? num_cols-1 : window_x < 0 ? 0 : window_x;
				window_y = window_y >= num_rows ? num_rows-1 : window_y < 0 ? 0 : window_y;
				point_data.signature[(template_y * 7) + template_x] = normalized_input_image[d_idx_1d(window_x, window_y, num_cols)];
			}
		}

		points[write_idx] = point_data;
	}
}

__device__ float calc_correlation(
	int search_area_x,
	int search_area_y,
	float * __restrict signature,
	float * __restrict normalized_input_image,
	uint num_cols,
	uint num_rows)
{
	const float template_average = d_get_template_average(signature);

	float ixy = 0.0f;
	float ix2 = 0.0f;
	float iy2 = 0.0f;
	for(char window_offset_y=-3, template_y=0; window_offset_y<=3; ++window_offset_y, ++template_y) {
		for(char window_offset_x=-3, template_x=0; window_offset_x<=3; ++window_offset_x, ++template_x) {
			int window_x = search_area_x + window_offset_x;
			int window_y = search_area_y + window_offset_y;
			window_x = window_x >= num_cols ? num_cols-1 : window_x < 0 ? 0 : window_x;
			window_y = window_y >= num_rows ? num_rows-1 : window_y < 0 ? 0 : window_y;

			float window_average = d_get_window_average(window_x, window_y, num_cols, num_rows, normalized_input_image);

			float pixel_value = normalized_input_image[d_idx_1d(window_x, window_y, num_cols)];
			float template_value = signature[(template_y * 7) + template_x];

			float ix = pixel_value - window_average;
			float iy = template_value - template_average;

			ixy += ix * iy;
			ix2 += ix * ix;
			iy2 += iy * iy;
		}
	}

	return ixy / sqrt(ix2 * iy2);
}

/* Evaluate correlation value of each pixel in an area around the current tracked feature */
__global__ void _calc_correlation_values(
	d_HarrisPoint * __restrict tracked_features,
	float * __restrict normalized_input_image,
	d_Correlation * __restrict correlation_map,
	d_Correlation * __restrict correlation_map_new_template,
	uint template_update_frames,
	uint num_cols,
	uint num_rows)
{
	const uint2 thread_2D_pos = make_uint2(
		threadIdx.x,
		threadIdx.y
	);
	const uint thread_1D_pos = d_idx_1d(thread_2D_pos.x, thread_2D_pos.y, 7);
	const uint feature_idx = blockIdx.x;
	const uint correlation_idx = (feature_idx * 49) + thread_1D_pos;

	d_HarrisPoint *feature = &tracked_features[feature_idx];

	int search_area_offset_x = thread_2D_pos.x - 3;
	int search_area_offset_y = thread_2D_pos.y - 3;
	
	/* Add offset to current tracked feature to get X and Y coordinates of point
	in the search area currently being evaluated for correlation */
	int search_area_x = feature->locations[feature->location_idx].x + search_area_offset_x;
	int search_area_y = feature->locations[feature->location_idx].y + search_area_offset_y;

	correlation_map[correlation_idx].location.x = search_area_x;
	correlation_map[correlation_idx].location.y = search_area_y;
	correlation_map_new_template[correlation_idx].location.x = search_area_x;
	correlation_map_new_template[correlation_idx].location.y = search_area_y;

	/* Return if this point in the window is outside the image */
	if(search_area_x>=num_cols || search_area_x<0 || search_area_y>=num_rows || search_area_y<0) {
		correlation_map[correlation_idx].correlation = -100000;
		correlation_map_new_template[correlation_idx].correlation = -100000;
		return;
	}

	/* Calculate and store correlation value for the current search area pixel */
	correlation_map[correlation_idx].correlation = calc_correlation(
		search_area_x, search_area_y,
		feature->signature,
		normalized_input_image,
		num_cols, num_rows
	);
	if((feature->track_frames + 1) % (template_update_frames * 2) == 0) {
		correlation_map_new_template[correlation_idx].correlation = calc_correlation(
			search_area_x, search_area_y,
			feature->new_signature,
			normalized_input_image,
			num_cols, num_rows
		);
	}
}

__device__ d_Correlation get_max_correlation(d_Correlation *correlation_map) {
	d_Correlation max_correlation;
	max_correlation.correlation = -100000;
	for(uchar i=0; i<49; ++i) {
		if(correlation_map[i].correlation > max_correlation.correlation) {
			max_correlation = correlation_map[i];
		}
	}
	return max_correlation;
}

__device__ __forceinline__ float d_distance(d_Point p1, d_Point p2) {
	const float diff_x = (long)p1.x - (long)p2.x;
	const float diff_y = (long)p1.y - (long)p2.y;
	return std::sqrt((diff_x * diff_x) + (diff_y * diff_y));
}

__global__ void _update_tracked_features(
	d_HarrisPoint * __restrict tracked_features,
	float * __restrict normalized_input_image,
	d_Correlation * __restrict correlation_map,
	d_Correlation * __restrict correlation_map_new_template,
	uint num_tracked,
	uint num_cols,
	uint num_rows,
	float correlation_threshhold,
	uint template_update_frames,
	float template_update_distance_threshhold)
{
	const uint idx = blockDim.x * blockIdx.x + threadIdx.x;

	if(idx >= num_tracked) {
		return;
	}

	d_HarrisPoint *feature = &tracked_features[idx];
	const uint correlation_idx = idx * 49;

	bool track_success;
	d_Point new_location;
	d_Correlation max_correlation = get_max_correlation(&correlation_map[correlation_idx]);

	if((feature->track_frames + 1) % (template_update_frames * 2) == 0) {
		d_Correlation max_correlation_new_template = get_max_correlation(&correlation_map_new_template[correlation_idx]);
		if(max_correlation_new_template.correlation >= correlation_threshhold && d_distance(max_correlation.location, max_correlation_new_template.location) < template_update_distance_threshhold) {
			new_location = max_correlation_new_template.location;
			track_success = true;
		} else {
			track_success = false;
		}
	} else {
		new_location = max_correlation.location;
		track_success = max_correlation.correlation >= correlation_threshhold;
	}

	if(track_success) {
		++feature->track_frames;

		feature->location_idx = (feature->location_idx + 1) % MAX_TRACKED_POINT_LOCATIONS;
		feature->locations[feature->location_idx] = new_location;

		/* If we have tracked this point for enough frames to trigger template updating */
		if(feature->track_frames % template_update_frames == 0 && (feature->track_frames + 1) % (template_update_frames * 2) != 0) {
			/* Update the tracked feature's 7x7 template to that of its current location in the image */
			for(char window_offset_y=-3, template_y=0; window_offset_y<=3; ++window_offset_y, ++template_y) {
				for(char window_offset_x=-3, template_x=0; window_offset_x<=3; ++window_offset_x, ++template_x) {
					int window_x = new_location.x + window_offset_x;
					int window_y = new_location.y + window_offset_y;
					window_x = window_x >= num_cols ? num_cols-1 : window_x < 0 ? 0 : window_x;
					window_y = window_y >= num_rows ? num_rows-1 : window_y < 0 ? 0 : window_y;

					feature->new_signature[(template_y * 7) + template_x] = normalized_input_image[d_idx_1d(window_x, window_y, num_cols)];
				}
			}
		}
	} else {
		/* Correlation value was not above threshhold
		feature has been lost and will no longer be tracked */
		feature->tracked = false;
	}
}

struct sort_by_corner_response {
	bool operator()(PointData const &left, PointData const &right) {
		return left.corner_response > right.corner_response;
	}
};

FeatureTrackingGpu::FeatureTrackingGpu(int cuda_device, const TrackingSettings &tracking_settings) :
	FeatureTracking(),
	device(cuda_device),
	settings(tracking_settings)
{
	cudaSetDevice(device);

	gradient_cols = image_width - 2;
	gradient_rows = image_height - 2;

	blur_gradient_cols = gradient_cols - (filter_range * 2);
	blur_gradient_rows = gradient_rows - (filter_range * 2);

	harris_response_cols = blur_gradient_cols;
	harris_response_rows = blur_gradient_rows;

	block_side_len = 32;
	block_size = dim3(block_side_len, block_side_len, 1);
	input_image_grid_size = dim3(image_width/block_side_len + 1, image_height/block_side_len + 1, 1);
	gradient_grid_size = dim3(gradient_cols/block_side_len + 1, gradient_rows/block_side_len + 1, 1);
	blur_gradient_grid_size = dim3(blur_gradient_cols/block_side_len + 1, blur_gradient_rows/block_side_len + 1, 1);
	harris_response_grid_size = dim3(harris_response_cols/block_side_len + 1, harris_response_rows/block_side_len + 1, 1);

	input_image_size = image_width * image_height * sizeof(uchar);

	checkCudaErrors(cudaMalloc(&d_input_image, input_image_size));
	checkCudaErrors(cudaMalloc(&d_normalized_input_image, image_width * image_height * sizeof(float)));
	checkCudaErrors(cudaMalloc(&d_tracked_feature_map, image_width * image_height * sizeof(bool)));
	checkCudaErrors(cudaMemset(d_tracked_feature_map, false, image_width * image_height * sizeof(bool)));
	checkCudaErrors(cudaMalloc(&d_sobel_x, 9 * sizeof(char)));
	checkCudaErrors(cudaMalloc(&d_sobel_y, 9 * sizeof(char)));
	checkCudaErrors(cudaMemcpy(d_sobel_x, sobel_x, 9 * sizeof(char), cudaMemcpyHostToDevice));
	checkCudaErrors(cudaMemcpy(d_sobel_y, sobel_y, 9 * sizeof(char), cudaMemcpyHostToDevice));
	checkCudaErrors(cudaMalloc(&d_gaussian_matrix, filter_width * filter_width * sizeof(float)));
	checkCudaErrors(cudaMemcpy(d_gaussian_matrix, gaussian_matrix, filter_width * filter_width * sizeof(float), cudaMemcpyHostToDevice));
	checkCudaErrors(cudaMalloc(&d_uchar_normalize_table, 256 * sizeof(float)));
	checkCudaErrors(cudaMemcpy(d_uchar_normalize_table, uchar_normalize_table, 256 * sizeof(float), cudaMemcpyHostToDevice));
	checkCudaErrors(cudaMalloc(&d_gradient_x2, gradient_cols * gradient_rows * sizeof(short)));
	checkCudaErrors(cudaMalloc(&d_gradient_y2, gradient_cols * gradient_rows * sizeof(short)));
	checkCudaErrors(cudaMalloc(&d_gradient_xy, gradient_cols * gradient_rows * sizeof(short)));
	checkCudaErrors(cudaMalloc(&d_blur_gradient_x2, blur_gradient_cols * blur_gradient_rows * sizeof(float)));
	checkCudaErrors(cudaMalloc(&d_blur_gradient_y2, blur_gradient_cols * blur_gradient_rows * sizeof(float)));
	checkCudaErrors(cudaMalloc(&d_blur_gradient_xy, blur_gradient_cols * blur_gradient_rows * sizeof(float)));
	checkCudaErrors(cudaMalloc(&d_harris_response, harris_response_cols * harris_response_rows * sizeof(float)));
	checkCudaErrors(cudaMalloc(&d_maxima_suppression, harris_response_cols * harris_response_rows * sizeof(bool)));
	checkCudaErrors(cudaMalloc(&d_points_after_suppression, sizeof(uint)));
	checkCudaErrors(cudaMalloc(&d_tracked_features, settings.max_tracked_features * sizeof(d_HarrisPoint)));
	checkCudaErrors(cudaMalloc(&d_correlation_map, settings.max_tracked_features * 49 * sizeof(d_Correlation)));
	checkCudaErrors(cudaMalloc(&d_correlation_map_new_template, settings.max_tracked_features * 49 * sizeof(d_Correlation)));

	h_tracked_features = (HarrisPoint *)malloc(settings.max_tracked_features * sizeof(HarrisPoint));
	h_tracked_feature_map = (bool *)malloc(image_width * image_height * sizeof(bool));
}

FeatureTrackingGpu::~FeatureTrackingGpu() {
	checkCudaErrors(cudaFree(d_input_image));
	checkCudaErrors(cudaFree(d_normalized_input_image));
	checkCudaErrors(cudaFree(d_tracked_feature_map));
	checkCudaErrors(cudaFree(d_sobel_x));
	checkCudaErrors(cudaFree(d_sobel_y));
	checkCudaErrors(cudaFree(d_gaussian_matrix));
	checkCudaErrors(cudaFree(d_uchar_normalize_table));
	checkCudaErrors(cudaFree(d_gradient_x2));
	checkCudaErrors(cudaFree(d_gradient_y2));
	checkCudaErrors(cudaFree(d_gradient_xy));
	checkCudaErrors(cudaFree(d_blur_gradient_x2));
	checkCudaErrors(cudaFree(d_blur_gradient_y2));
	checkCudaErrors(cudaFree(d_blur_gradient_xy));
	checkCudaErrors(cudaFree(d_harris_response));
	checkCudaErrors(cudaFree(d_maxima_suppression));
	checkCudaErrors(cudaFree(d_points_after_suppression));
	checkCudaErrors(cudaFree(d_tracked_features));
	checkCudaErrors(cudaFree(d_correlation_map));
	checkCudaErrors(cudaFree(d_correlation_map_new_template));

	free(h_tracked_features);
	free(h_tracked_feature_map);
}

void FeatureTrackingGpu::create_normalised_input_image() {
	_create_normalised_input_image<<<input_image_grid_size, block_size>>>(
		d_input_image,
		d_uchar_normalize_table,
		d_normalized_input_image,
		image_width,
		image_height
	);
}

void FeatureTrackingGpu::calc_gradients() {
	_calc_gradients<<<gradient_grid_size, block_size>>>(
		d_input_image,
		d_gradient_x2,
		d_gradient_y2,
		d_gradient_xy,
		d_sobel_x,
		d_sobel_y,
		image_width,
		gradient_cols,
		gradient_rows
	);
}

void FeatureTrackingGpu::blur_gradients() {
	_blur_gradients<<<blur_gradient_grid_size, block_size>>>(
		d_gradient_x2,
		d_blur_gradient_x2,
		d_gaussian_matrix,
		filter_range,
		gradient_cols,
		blur_gradient_cols,
		blur_gradient_rows
	);

	_blur_gradients<<<blur_gradient_grid_size, block_size>>>(
		d_gradient_y2,
		d_blur_gradient_y2,
		d_gaussian_matrix,
		filter_range,
		gradient_cols,
		blur_gradient_cols,
		blur_gradient_rows
	);

	_blur_gradients<<<blur_gradient_grid_size, block_size>>>(
		d_gradient_xy,
		d_blur_gradient_xy,
		d_gaussian_matrix,
		filter_range,
		gradient_cols,
		blur_gradient_cols,
		blur_gradient_rows
	);
}

void FeatureTrackingGpu::calc_harris_response() {
	_calc_harris_response<<<blur_gradient_grid_size, block_size>>>(
		d_blur_gradient_x2,
		d_blur_gradient_y2,
		d_blur_gradient_xy,
		d_harris_response,
		settings.sensitivity,
		blur_gradient_cols,
		blur_gradient_rows
	);
}

void FeatureTrackingGpu::get_maxima_points() {
	checkCudaErrors(cudaMemset(d_maxima_suppression, 1, harris_response_cols * harris_response_rows * sizeof(bool)));
	checkCudaErrors(cudaMemset(d_points_after_suppression, 0, sizeof(uint)));
	_non_maximum_suppression<<<harris_response_grid_size, block_size>>>(
		d_harris_response,
		d_maxima_suppression,
		maxima_suppression_range,
		settings.harris_response_threshhold,
		harris_response_cols,
		harris_response_rows,
		d_points_after_suppression
	);
	checkCudaErrors(cudaDeviceSynchronize());

	uint num_points;
	checkCudaErrors(cudaMemcpy(
		&num_points,
		d_points_after_suppression,
		sizeof(uint),
		cudaMemcpyDeviceToHost
	));

	d_PointData *d_points;
	checkCudaErrors(cudaMalloc(&d_points, num_points * sizeof(d_PointData)));
	checkCudaErrors(cudaMemset(d_points_after_suppression, 0, sizeof(uint)));
	_fill_points<<<harris_response_grid_size, block_size>>>(
		d_harris_response,
		d_maxima_suppression,
		d_points,
		filter_range,
		harris_response_cols,
		harris_response_rows,
		image_width,
		image_height,
		d_normalized_input_image,
		d_points_after_suppression
	);
	checkCudaErrors(cudaDeviceSynchronize());

	PointData *h_points = (PointData *)malloc(num_points * sizeof(PointData));
	cudaMemcpy(h_points, d_points, num_points * sizeof(PointData), cudaMemcpyDeviceToHost);
	checkCudaErrors(cudaFree(d_points));

	std::sort(&h_points[0], &h_points[num_points-1], sort_by_corner_response());

	harris_points.clear();
#pragma loop(hint_parallel(MAX_AP_THREADS))
	for(uint i=0; i<settings.max_tracked_features && i<num_points; ++i) {
		HarrisPoint harris_point;
		harris_point.locations[0] = h_points[i].location;
		memcpy(harris_point.signature, h_points[i].signature, 49 * sizeof(float));
		++harris_point.track_frames;
		harris_point.tracked = true;
		harris_points.push_back(harris_point);
	}

	free(h_points);
}

void FeatureTrackingGpu::update_tracked_features() {
	if(image_count == 0) {
		tracked_features = harris_points;
		return;
	}

	const size_t num_tracked = tracked_features.size();

	if(num_tracked == 0) {
		return;
	}

	checkCudaErrors(cudaMemcpy(d_tracked_features, &tracked_features[0], num_tracked * sizeof(HarrisPoint), cudaMemcpyHostToDevice));

	_calc_correlation_values<<<dim3(num_tracked, 1, 1), dim3(7, 7, 1)>>>(
		d_tracked_features,
		d_normalized_input_image,
		d_correlation_map,
		d_correlation_map_new_template,
		settings.template_update_frames,
		image_width,
		image_height
	);
	checkCudaErrors(cudaDeviceSynchronize());

	_update_tracked_features<<<1, num_tracked>>>(
		d_tracked_features,
		d_normalized_input_image,
		d_correlation_map,
		d_correlation_map_new_template,
		num_tracked,
		image_width,
		image_height,
		settings.correlation_threshhold,
		settings.template_update_frames,
		settings.template_update_distance_threshhold
	);
	checkCudaErrors(cudaDeviceSynchronize());

	checkCudaErrors(cudaMemcpy(h_tracked_features, d_tracked_features, num_tracked * sizeof(HarrisPoint), cudaMemcpyDeviceToHost));

	memset(h_tracked_feature_map, 0, image_width * image_height * sizeof(bool));
	tracked_features.clear();
#pragma loop(hint_parallel(MAX_AP_THREADS))
	for(size_t i=0; i<num_tracked; ++i) {
		if(h_tracked_features[i].tracked) {
			const uint x = h_tracked_features[i].locations[h_tracked_features[i].location_idx].x;
			const uint y = h_tracked_features[i].locations[h_tracked_features[i].location_idx].y;
			h_tracked_feature_map[idx_1d(x, y, image_width)];
			tracked_features.push_back(h_tracked_features[i]);
		}
	}

	const uint num_harris_points = harris_points.size();
#pragma loop(hint_parallel(MAX_AP_THREADS))
	for(size_t i=0; (i<num_harris_points) && (tracked_features.size()<settings.max_tracked_features); ++i) {
		const uint x = harris_points[i].locations[harris_points[i].location_idx].x;
		const uint y = harris_points[i].locations[harris_points[i].location_idx].y;
		for(char window_offset_y=-3; window_offset_y<=3; ++window_offset_y) {
			for(char window_offset_x=-3; window_offset_x<=3; ++window_offset_x) {
				int window_x = x + window_offset_x;
				int window_y = y + window_offset_y;
				window_x = window_x >= image_width ? image_width-1 : window_x < 0 ? 0 : window_x;
				window_y = window_y >= image_height ? image_height-1 : window_y < 0 ? 0 : window_y;

				if(h_tracked_feature_map[idx_1d(window_x, window_y, image_width)] == true) {
					goto NEXT_POINT;
				}
			}
		}
		tracked_features.push_back(harris_points[i]);
		h_tracked_feature_map[idx_1d(x, y, image_width)] = true;
		if(tracked_features.size() >= settings.max_tracked_features) {
			break;
		}
NEXT_POINT:;
	}
}

std::vector<HarrisPoint> FeatureTrackingGpu::feature_points(uchar *input) {
	h_input_image = input;

	checkCudaErrors(cudaMemcpy(d_input_image, h_input_image, input_image_size, cudaMemcpyHostToDevice));

	create_normalised_input_image();
	calc_gradients();
	checkCudaErrors(cudaDeviceSynchronize());

	blur_gradients();
	checkCudaErrors(cudaDeviceSynchronize());

	calc_harris_response();
	checkCudaErrors(cudaDeviceSynchronize());

	get_maxima_points();
	update_tracked_features();

	++image_count;
	
	return tracked_features;
}
