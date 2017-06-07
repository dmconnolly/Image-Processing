#include <algorithm>
#include <vector>

#include <QtCharts\qlineseries.h>
#include <QtCharts\qchart.h>

#include "Utils/utils.hpp"
#include "Utils/colours.hpp"
#include "Tracking/Cpu/feature_tracking_cpu.hpp"
#include "Tracking/Gpu/feature_tracking_gpu.cuh"

#include "controller.hpp"

Controller::Controller(Ui::GuiClass &ui_ref) :
	ui(ui_ref),
	pangu(&steps)
{
	init_gui_chart();

	connect(this, SIGNAL(updateUiRequest(QImage, uint, uint)), this, SLOT(onUpdateUiRequest(QImage, uint, uint)));
	connect(this, SIGNAL(finishedProcessing(void)), this, SLOT(onFinishedProcessing(void)));

	processed_image_size = image_width * image_height * sizeof(uchar) * 3;
	processed_image = (uchar *)malloc(processed_image_size);
}

Controller::~Controller() {
	stop_processing();
	free(processed_image);
}

void Controller::init_gui_chart() {
	tracking_times_chart = new QtCharts::QChart();

	QFont chart_title_font;
	chart_title_font.setPixelSize(16);
	tracking_times_chart->setTitleFont(chart_title_font);
	tracking_times_chart->setTitleBrush(QBrush(QRgb(0xCCCCCC)));
	tracking_times_chart->setTitle("Frame timings");
	tracking_times_chart->setBackgroundBrush(QBrush(QColor(25, 25, 25)));
	tracking_times_chart->legend()->setLabelColor(QRgb(0xCCCCCC));

	cpu_tracking_series = new QtCharts::QLineSeries;
	QPen cpu_series_pen;
	cpu_series_pen.setColor(cpu_colour);
	cpu_series_pen.setWidth(2);
	cpu_tracking_series->setPen(cpu_series_pen);
	cpu_tracking_series->setName("CPU");

	gpu_tracking_series = new QtCharts::QLineSeries;
	QPen gpu_series_pen;
	gpu_series_pen.setColor(gpu_colour);
	gpu_series_pen.setWidth(2);
	gpu_tracking_series->setPen(gpu_series_pen);
	gpu_tracking_series->setName("GPU");

	tracking_times_chart->addSeries(cpu_tracking_series);
	tracking_times_chart->addSeries(gpu_tracking_series);

	QFont labels_font;
	labels_font.setPixelSize(14);

	chart_axis_x = new QValueAxis;
	chart_axis_x->setTitleText("Frame number");
	chart_axis_x->setTitleBrush(QBrush(QRgb(0xCCCCCC)));
	chart_axis_x->setTickCount(11);
	chart_axis_x->setRange(0, min_chart_x);
	chart_axis_x->setLabelFormat("%u");
	chart_axis_x->setGridLineColor(QRgb(0x333333));
	chart_axis_x->setLabelsBrush(QBrush(QRgb(0xCCCCCC)));
	chart_axis_x->setLabelsFont(labels_font);
	tracking_times_chart->addAxis(chart_axis_x, Qt::AlignBottom);

	chart_axis_y = new QLogValueAxis;
	chart_axis_y->setLabelFormat("%u");
	chart_axis_y->setTitleText("Frame time (ms)");
	chart_axis_y->setTitleBrush(QBrush(QRgb(0xCCCCCC)));
	chart_axis_y->setBase(2);
	chart_axis_y->setMax(min_chart_y);
	chart_axis_y->setGridLineColor(QRgb(0x333333));
	chart_axis_y->setLabelsBrush(QBrush(QRgb(0xCCCCCC)));
	chart_axis_y->setLabelsFont(labels_font);
	tracking_times_chart->addAxis(chart_axis_y, Qt::AlignLeft);

	cpu_tracking_series->attachAxis(chart_axis_x);
	cpu_tracking_series->attachAxis(chart_axis_y);
	gpu_tracking_series->attachAxis(chart_axis_x);
	gpu_tracking_series->attachAxis(chart_axis_y);

	ui.trackingTimesChartView->setChart(tracking_times_chart);
}

void Controller::start_processing() {
	if(!running) {
		update_settings();
		running = true;
		stop = false;
		processing_thread = std::thread(&Controller::_start_processing, this);
	}
}

void Controller::_start_processing() {
	max_frame_count = 0;
	max_frame_time_ms = 0;

	cpu_tracking_times.frame_times_ms.clear();
	cpu_tracking_times.total_ms = 0;
	cpu_tracking_times.max_frame_time_ms = 0;

	gpu_tracking_times.frame_times_ms.clear();
	gpu_tracking_times.total_ms = 0;
	gpu_tracking_times.max_frame_time_ms = 0;

	cpu_tracking_series->clear();
	gpu_tracking_series->clear();

	cpu_frame = 0;
	gpu_frame = 0;

	emit updateUiRequest(QImage(), cpu_frame, gpu_frame);

	steps = PanguServer::read_pangu_steps(flight_file_path);
	settings.max_frames = std::min(settings.max_frames, (uint)steps.size());

	pangu.start(settings.max_frames);
	feature_tracking(&(FeatureTrackingCpu(settings)), cpu_frame, cpu_tracking_times, cpu_pen_bgr);
	pangu.stop();

	pangu.start(settings.max_frames);
	feature_tracking(&(FeatureTrackingGpu(cuda_device, settings)), gpu_frame, gpu_tracking_times, gpu_pen_bgr);
	pangu.stop();

	emit finishedProcessing();
}

void Controller::feature_tracking(FeatureTracking *tracking, uint &frame_counter, ProcessingTimes &times, Colour pen_colour) {
	for(frame_counter=1; frame_counter<=settings.max_frames && !stop; ++frame_counter) {
		uchar *original_image = pangu.get_image(5000);
		if(!original_image) {
			break;
		}
		gray_arr_to_rgb_mat(&original_image[pangu.image_offset], processed_image, image_width, image_height);

		std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
		std::vector<HarrisPoint> feature_points = tracking->feature_points(&original_image[pangu.image_offset]);
		std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();
		double duration_ms = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count() / 1000.0;
		times.frame_times_ms.push_back(duration_ms);
		times.total_ms += duration_ms;

		mark_feature_points(processed_image, feature_points, image_width, image_height, 1, pen_colour);

		QImage tmp = QImage(processed_image, image_width, image_height, QImage::Format::Format_RGB888);
		emit updateUiRequest(
			tmp.copy(),
			(uint)(((float)cpu_frame / (settings.max_frames) * 100.0f)),
			(uint)(((float)gpu_frame / (settings.max_frames) * 100.0f))
		);

		free(original_image);
	}
}

void Controller::stop_processing() {
	if(running) {
		stop = true;

		if(processing_thread.joinable()) {
			processing_thread.join();
		}

		pangu.stop();
		running = false;
	}
}

void Controller::update_settings() {
	settings.max_frames = ui.maximumFramesSpinBox->value();
	settings.max_tracked_features = ui.maxTrackedSpinBox->value();
	settings.sensitivity = ui.sensitivitySpinBox->value();
	settings.harris_response_threshhold = ui.harrisThreshholdSpinBox->value();
	settings.correlation_threshhold = ui.correlationThreshholdSpinBox->value();
	settings.template_update_frames = ui.templateUpdateFramesSpinBox->value();
	settings.template_update_distance_threshhold = ui.templateUpdateMaximumDistanceSpinBox->value();
}

void Controller::onUpdateUiRequest(QImage q_image, uint cpu_progress, uint gpu_progress) {
	ui.imageDisplayLabel->setPixmap(QPixmap::fromImage(q_image));

	ui.cpuProgressBar->setValue(cpu_progress);
	ui.gpuProgressBar->setValue(gpu_progress);

	update_gui_chart();
	update_gui_stats();
}

void Controller::update_gui_chart() {
	for(uint i=cpu_tracking_series->count(); i<cpu_tracking_times.frame_times_ms.size(); ++i) {
		cpu_tracking_series->append(i, cpu_tracking_times.frame_times_ms[i]);
		if(cpu_tracking_times.frame_times_ms[i] > cpu_tracking_times.max_frame_time_ms) {
			cpu_tracking_times.max_frame_time_ms = cpu_tracking_times.frame_times_ms[i];
		}
	}

	for(uint i=gpu_tracking_series->count(); i<gpu_tracking_times.frame_times_ms.size(); ++i) {
		gpu_tracking_series->append(i, gpu_tracking_times.frame_times_ms[i]);
		if(gpu_tracking_times.frame_times_ms[i] > gpu_tracking_times.max_frame_time_ms) {
			gpu_tracking_times.max_frame_time_ms = gpu_tracking_times.frame_times_ms[i];
		}
	}

	if(cpu_tracking_times.max_frame_time_ms > max_frame_time_ms || gpu_tracking_times.max_frame_time_ms > max_frame_time_ms) {
		max_frame_time_ms = std::max(cpu_tracking_times.max_frame_time_ms, gpu_tracking_times.max_frame_time_ms);
		chart_axis_y->setMax(max_frame_time_ms + (max_frame_time_ms));
	}

	if(cpu_tracking_series->count() > max_frame_count || gpu_tracking_series->count() > max_frame_count) {
		max_frame_count = std::min(max_frame_count + min_chart_x, settings.max_frames);
		chart_axis_x->setMax(max_frame_count);
	}
}

void Controller::update_gui_stats() {
	QString text;

	uint cpu_num_frames = cpu_tracking_times.frame_times_ms.size();
	if(cpu_num_frames == 0) {
		ui.cpuFramesProcessedLabel->setText("0");
		ui.cpuAverageFrameTimeLabel->setText("N/A");
		ui.cpuMaxFrameTimeLabel->setText("N/A");
		ui.cpuTotalProcessingTimeLabel->setText("N/A");
	} else {
		text.sprintf("%u", cpu_num_frames);
		ui.cpuFramesProcessedLabel->setText(text);
		text.sprintf("%.2f ms", cpu_tracking_times.total_ms/cpu_num_frames);
		ui.cpuAverageFrameTimeLabel->setText(text);
		text.sprintf("%.2f ms", cpu_tracking_times.max_frame_time_ms);
		ui.cpuMaxFrameTimeLabel->setText(text);
		text.sprintf("%.2f seconds", cpu_tracking_times.total_ms/1000);
		ui.cpuTotalProcessingTimeLabel->setText(text);
	}

	uint gpu_num_frames = gpu_tracking_times.frame_times_ms.size();
	if(gpu_num_frames == 0) {
		ui.gpuFramesProcessedLabel->setText("0");
		ui.gpuAverageFrameTimeLabel->setText("N/A");
		ui.gpuMaxFrameTimeLabel->setText("N/A");
		ui.gpuTotalProcessingTimeLabel->setText("N/A");
	} else {
		text.sprintf("%u", gpu_num_frames);
		ui.gpuFramesProcessedLabel->setText(text);
		text.sprintf("%.2f ms", gpu_tracking_times.total_ms/gpu_num_frames);
		ui.gpuAverageFrameTimeLabel->setText(text);
		text.sprintf("%.2f ms", gpu_tracking_times.max_frame_time_ms);
		ui.gpuMaxFrameTimeLabel->setText(text);
		text.sprintf("%.2f seconds", gpu_tracking_times.total_ms/1000);
		ui.gpuTotalProcessingTimeLabel->setText(text);
	}
}

void Controller::onFinishedProcessing() {
	stop_processing();
	ui.startButton->setEnabled(true);
	ui.stopButton->setDisabled(true);
}

void Controller::set_flight_file_path(QString path) {
	flight_file_path = path.toStdString();
}
