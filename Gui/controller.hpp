#pragma once
#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <vector>
#include <chrono>

#include <QtCharts\qlineseries.h>
#include <QtCharts\QLogValueAxis>
#include <QtCharts\QValueAxis>

#include "Pangu/pangu_server.hpp"
#include "Utils/utils.hpp"
#include "Tracking/feature_tracking.hpp"
#include "ui_gui.h"

struct ProcessingTimes {
	std::vector<double> frame_times_ms;
	double max_frame_time_ms = 0;
	double total_ms = 0;
};

class Controller : QObject {
	Q_OBJECT

public:
	Controller(Ui::GuiClass &ui);
	~Controller();
	void start_processing();
	void stop_processing();
	void set_flight_file_path(QString path);

private:
	const uint image_width = 1024;
	const uint image_height = 768;
	const int cuda_device = 1;

	Ui::GuiClass &ui;
	PanguServer pangu;
	std::vector<PanguStep> steps;
	TrackingSettings settings;
	size_t processed_image_size;
	uchar *processed_image;
	bool running = false;
	std::atomic<bool> stop = false;
	std::thread processing_thread;
	uint cpu_frame;
	uint gpu_frame;
	ProcessingTimes cpu_tracking_times;
	ProcessingTimes gpu_tracking_times;
	QtCharts::QLineSeries *cpu_tracking_series;
	QtCharts::QLineSeries *gpu_tracking_series;
	QtCharts::QValueAxis *chart_axis_x;
	QtCharts::QLogValueAxis *chart_axis_y;
	QtCharts::QChart *tracking_times_chart;
	std::string flight_file_path;

	const uint min_chart_x = 10;
	const uint min_chart_y = 100;
	uint max_frame_count = 10;
	double max_frame_time_ms = 100;

	void update_settings();
	void _start_processing();
	void feature_tracking(FeatureTracking *tracking, uint &frame_counter, ProcessingTimes &times, Colour pen_colour);
	void init_gui_chart();
	void update_gui_chart();
	void update_gui_stats();

signals:
	void updateUiRequest(QImage, uint, uint);
	void finishedProcessing(void);

private slots:
	void onUpdateUiRequest(QImage, uint, uint);
	void onFinishedProcessing(void);
};

#endif /* CONTROLLER_HPP */
