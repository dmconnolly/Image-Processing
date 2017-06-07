#include <qfiledialog.h>

#include "Utils/colours.hpp"

#include "gui.h"

Gui::Gui(QWidget *parent) : 
	QMainWindow(parent)
{
	ui.setupUi(this);

	QPalette palette;

	ui.tabWidget->setStyleSheet("background-color: #222222;");

	ui.cpuFramesProcessedLabel->setStyleSheet("QLabel { color : #" + cpu_colour_str + "; }");
	ui.cpuAverageFrameTimeLabel->setStyleSheet("QLabel { color : #" + cpu_colour_str + "; }");
	ui.cpuMaxFrameTimeLabel->setStyleSheet("QLabel { color : #" + cpu_colour_str + "; }");
	ui.cpuTotalProcessingTimeLabel->setStyleSheet("QLabel { color : #" + cpu_colour_str + "; }");

	palette = ui.cpuProgressBar->palette();
	palette.setBrush(QPalette::Highlight, QBrush(cpu_colour));
	ui.cpuProgressBar->setPalette(palette);

	ui.gpuFramesProcessedLabel->setStyleSheet("QLabel { color : #" + gpu_colour_str + "; }");
	ui.gpuAverageFrameTimeLabel->setStyleSheet("QLabel { color : #" + gpu_colour_str + "; }");
	ui.gpuMaxFrameTimeLabel->setStyleSheet("QLabel { color : #" + gpu_colour_str + "; }");
	ui.gpuTotalProcessingTimeLabel->setStyleSheet("QLabel { color : #" + gpu_colour_str + "; }");

	palette = ui.gpuProgressBar->palette();
	palette.setBrush(QPalette::Highlight, QBrush(gpu_colour));
	ui.gpuProgressBar->setPalette(palette);

	connect(ui.startButton, SIGNAL(clicked()), this, SLOT(onStartButtonClick()));
	connect(ui.stopButton, SIGNAL(clicked()), this, SLOT(onStopButtonClick()));
	connect(ui.openFlightFileButton, SIGNAL(clicked()), this, SLOT(onOpenFlightFileButtonClick()));
	controller = new Controller(ui);
}

Gui::~Gui() {
	delete controller;
}

void Gui::onStartButtonClick() {
	ui.startButton->setDisabled(true);
	ui.stopButton->setEnabled(true);
	ui.openFlightFileButton->setDisabled(false);
	controller->start_processing();
}

void Gui::onStopButtonClick() {
	controller->stop_processing();
	ui.startButton->setEnabled(true);
	ui.stopButton->setDisabled(true);
	ui.openFlightFileButton->setDisabled(false);
}

void Gui::onOpenFlightFileButtonClick() {
	QString filename = QFileDialog::getOpenFileName(
		this,
		tr("Open PANGU flight file"),
		"./Flights",
		tr("fli Files (*.fli)")
	);

	if(filename.size() > 0) {
		controller->set_flight_file_path(filename);
		ui.startButton->setEnabled(true);
	}
}
