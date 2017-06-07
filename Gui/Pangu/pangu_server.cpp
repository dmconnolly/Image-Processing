#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdio.h>

#include "pangu_server.hpp"
#include "pan_protocol_lib.h"

PanguServer::PanguServer(std::vector<PanguStep> *steps) :
	steps(steps),
	image_queue(max(max_image_queue_size, 1)),
	exit(true)
{
	/* Empty */
}

PanguServer::~PanguServer() {
	stop();
}

void PanguServer::start(uint max_frames) {
	_connect();

	pan_protocol_get_camera_properties(
		sock, 0,
		&image_width,
		&image_height,
		NULL, NULL, NULL,
		NULL, NULL, NULL,
		NULL, NULL, NULL
	);

	ulong image_size_bytes;
	uchar *image = pan_protocol_get_image(sock, &image_size_bytes);
	single_img_size_bytes = image_size_bytes;
	image_offset = image_start_offset(image);
	free(image);

	this->max_frames = max_frames;
	step_idx = 0;
	exit = false;
	gen_thread = std::thread(&PanguServer::generate_images, this);
}

void PanguServer::stop() {
	exit = true;

	if(gen_thread.joinable()) {
		gen_thread.join();
	}

	uchar *image = nullptr;
	while(image_queue.try_dequeue(image)) {
		free(image);
	}

	_disconnect();
}

unsigned char * PanguServer::get_image(uint ms) {
	uchar *image = 0;
	image_queue.wait_dequeue_timed(image, std::chrono::milliseconds(ms));
	return image;
}

ulong PanguServer::host_id_to_address(char *s) {
	struct hostent *host;

	/* Assume we have a dotted IP address ... */
	long result = inet_addr(s);
	if(result != INADDR_NONE) {
		return result;
	}

	/* That failed so assume DNS will resolve it. */
	host = gethostbyname(s);
	return host ? *((long *)host->h_addr_list[0]) : INADDR_NONE;
}

void PanguServer::_connect() {
	WSAData wsaData;
	if(WSAStartup(MAKEWORD(1, 1), &wsaData)) {
		throw std::runtime_error("Failed to initialise winsock 1.1");
	}

	/* Get IP address of the server */
	long ip_addr = host_id_to_address(server_name);

	/* Create a TCP/IP socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1) {
		throw std::runtime_error("Failed to create socket");
	}

	/* Connect the socket to the remote server */
	struct sockaddr_in sock_addr;
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.s_addr = ip_addr;
	sock_addr.sin_port = htons(server_port);
	ulong sock_addr_len = sizeof(struct sockaddr_in);
	if(connect(sock, (struct sockaddr *)&sock_addr, sock_addr_len) == -1) {
		throw std::runtime_error("Failed to connect to server");
	}

	/* Start the PANGU network communications protocol */
	pan_protocol_start(sock);
}

void PanguServer::_disconnect() {
	pan_protocol_finish(sock);
	SOCKET_CLOSE(sock);
	WSACleanup();
}

size_t PanguServer::image_start_offset(uchar *image) {
	size_t offset = 0;
	for(char newlines=0; newlines<2; newlines+=image[offset++]=='\n');
	return offset;
}

void PanguServer::generate_images() {
	const uint num_steps = steps->size();
	const long long max_step_idx = ((long long)min(num_steps, max_frames)) - 1;
	bool image_enqueue_fail = false;
	uchar *image;
	while(step_idx <= max_step_idx) {
		/* Wait until new images are required */
		while(!exit && image_queue.size_approx() >= max_image_queue_size) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}

		if(exit) {
			return;
		}

		if(!image_enqueue_fail) {
			/* Get the current step */
			PanguStep &step = (*steps)[step_idx];

			/* Update camera position */
			pan_protocol_set_viewpoint_by_degrees_d(
				sock, step.x, step.y, step.z,
				step.yaw, step.pitch, step.roll
			);

			/* Add new image pointer to the queue */
			ulong image_size_bytes;
			image = pan_protocol_get_image(sock, &image_size_bytes);
			if(!image) {
				printf("Failed to get an image from pangu");
				throw std::runtime_error("Failed to get an image from pangu");
			}
		}

		if(!image_queue.try_enqueue(image)) {
			image_enqueue_fail = true;
			continue;
		} else {
			image_enqueue_fail = false;
			++step_idx;
		}
	}
}

std::vector<PanguStep> PanguServer::read_pangu_steps(std::string flight_file_path) {
	std::vector<PanguStep> steps;

	std::ifstream flight_file_stream(flight_file_path);

	std::string line;
	while(std::getline(flight_file_stream, line)) {
		std::istringstream iss(line);

		std::string start_token;
		if(!(iss >> start_token) || start_token != "start") {
			continue;
		}

		PanguStep step;
		iss >> step.x;
		iss >> step.y;
		iss >> step.z;
		iss >> step.yaw;
		iss >> step.pitch;
		iss >> step.roll;

		steps.push_back(step);
	}

	return steps;
}