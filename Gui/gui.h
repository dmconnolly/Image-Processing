#pragma once
#ifndef GUI_HPP
#define GUI_HPP

#include <QtWidgets/QMainWindow>

#include "ui_gui.h"
#include "controller.hpp"

class Gui : public QMainWindow {
    Q_OBJECT

public:
    Gui(QWidget *parent = Q_NULLPTR);
	~Gui();

public slots:
	void onStartButtonClick();
	void onStopButtonClick();
	void onOpenFlightFileButtonClick();

private:
    Ui::GuiClass ui;
	Controller *controller;
};

#endif /* GUI_HPP */
