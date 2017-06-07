#define _USE_MATH_DEFINES
#include <math.h>

#include "flight_writer.hpp"

double FlightWriter::deg_to_rad(double degrees) {
	return M_PI * (degrees / 180);
}

void FlightWriter::write_step(std::ofstream &fh, PanguStep step) {
	fh << "start " <<
		step.x << " " <<
		step.y << " " <<
		step.z << " " <<
		step.yaw << " " <<
		step.pitch << " " <<
		step.roll << "\n";
}

void FlightWriter::interpolate(std::string file_path, uint frames, PanguStep start, PanguStep end) {
	std::ofstream fh(file_path);

	PanguStep step(
		(end.x - start.x) / frames,
		(end.y - start.y) / frames,
		(end.z - start.z) / frames,
		(end.yaw - start.yaw) / frames,
		(end.pitch - start.pitch) / frames,
		(end.roll - start.roll) / frames
	);
	PanguStep current = start;

	fh << "view craft\n";

	for(uint i=0; i<frames; ++i) {
		write_step(fh, current);

		current = PanguStep(
			current.x + step.x,
			current.y + step.y,
			current.z + step.z,
			current.yaw + step.yaw,
			current.pitch + step.pitch,
			current.roll + step.roll
		);
	}
	fh.close();
}

void FlightWriter::orbit_equator(std::string file_path, uint frames, Point target, double distance, double start_azimuth, double azimuth_mod) {
	std::ofstream fh(file_path);

	const double azimuth_step = azimuth_mod / frames;

	fh << "view craft\n";

	for(uint i=0; i<frames; ++i) {
		const double mod_azimuth = fmod(start_azimuth + (azimuth_step * i), 360);

		PanguStep step(
			target.x + (distance * sin(deg_to_rad(mod_azimuth))),
			target.y + (distance * cos(deg_to_rad(mod_azimuth))),
			target.z,
			360 - (fmod(mod_azimuth + 180, 360)),
			0,
			0
		);
		write_step(fh, step);
	}

	fh.close();
}
