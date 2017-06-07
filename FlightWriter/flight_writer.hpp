#pragma once
#ifndef FLIGHT_WRITER_HPP
#define FLIGHT_WRITER_HPP

#include <iostream>
#include <fstream>
#include <string>

typedef unsigned int uint;

struct PanguStep {
	double x, y, z, yaw, pitch, roll;
	PanguStep() {

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

struct Point {
	double x, y, z;
	Point(double x, double y, double z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
};

class FlightWriter {
private:
	FlightWriter() {};
	~FlightWriter() {};
	static double deg_to_rad(double degrees);
	static void write_step(std::ofstream &fh, PanguStep step);

public:
	static void interpolate(std::string file_path, uint frames, PanguStep start, PanguStep end);
	static void orbit_equator(std::string file_path, uint frames, Point target, double distance, double start_azimuth, double azimuth_mod);
};

#endif /* FLIGHT_WRTITER_HPP */