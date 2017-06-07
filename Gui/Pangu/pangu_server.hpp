#pragma once
#ifndef PANGU_SERVER_HPP
#define PANGU_SERVER_HPP

#include <WinSock2.h>

#include <thread>
#include <vector>

#include "queue/atomicops.h"
#include "queue/readerwriterqueue.h"

#include "Utils/types.hpp"

/* For BlockingReaderWriterQueue */
using namespace moodycamel;

struct PanguStep {
	double x, y, z, yaw, pitch, roll;

	PanguStep() {
		/* Empty */
	}

	PanguStep(double x, double y, double z, double yaw, double pitch, double roll) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->yaw = yaw;
		this->pitch = pitch;
		this->roll = roll;
	}
};

class PanguServer {
public:
	size_t image_offset;
	ulong image_width;
	ulong image_height;
	BlockingReaderWriterQueue<uchar *> image_queue;
	uint max_image_queue_size = 200;

	PanguServer(std::vector<PanguStep> *steps);
	~PanguServer();
	void start(uint max_frames);
	void stop();
	uchar * get_image(uint ms);
	static std::vector<PanguStep> read_pangu_steps(std::string flight_file_path);
private:
	char server_name[11] { "localhost" };
	ushort server_port = 10363;

	SOCKET sock;
	size_t single_img_size_bytes;
	std::thread gen_thread;
	std::atomic<bool> exit = false;

	uint max_frames = 0;
	long long step_idx = 0;
	std::vector<PanguStep> *steps;

	void _connect();
	void _disconnect();
	void generate_images();
	static ulong host_id_to_address(char *s);
	static size_t image_start_offset(uchar *image);
};

#endif /* PANGU_SERVER_HPP */
