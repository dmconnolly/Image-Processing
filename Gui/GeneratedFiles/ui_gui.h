/********************************************************************************
** Form generated from reading UI file 'gui.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GUI_H
#define UI_GUI_H

#include <QtCharts/QChartView>

#include <QtCharts/chartsnamespace.h>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GuiClass
{
public:
    QWidget *centralWidget;
    QTableView *tableView;
    QGroupBox *featureDetectionSettingsGroupBox;
    QWidget *gridLayoutWidget_2;
    QGridLayout *gridLayout_2;
    QSpinBox *maxTrackedSpinBox;
    QLabel *sensitivityLabel;
    QLabel *maxTrackedLabel;
    QDoubleSpinBox *sensitivitySpinBox;
    QLabel *harrisThreshholdLabel;
    QSpinBox *harrisThreshholdSpinBox;
    QGroupBox *progressGroupBox;
    QProgressBar *cpuProgressBar;
    QProgressBar *gpuProgressBar;
    QLabel *label_7;
    QLabel *label_11;
    QGroupBox *featureTrackingSettingsGroupBox;
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QDoubleSpinBox *correlationThreshholdSpinBox;
    QLabel *correlationThreshholdLabel;
    QSpinBox *templateUpdateFramesSpinBox;
    QLabel *templateUpdateFramesLabel;
    QLabel *label_12;
    QDoubleSpinBox *templateUpdateMaximumDistanceSpinBox;
    QGroupBox *groupBox;
    QPushButton *startButton;
    QPushButton *stopButton;
    QGroupBox *groupBox_2;
    QWidget *gridLayoutWidget_3;
    QGridLayout *gridLayout_3;
    QLabel *label;
    QSpinBox *maximumFramesSpinBox;
    QPushButton *openFlightFileButton;
    QTabWidget *tabWidget;
    QWidget *imageDisplayTab;
    QLabel *imageDisplayLabel;
    QWidget *statisticsTab;
    QtCharts::QChartView *trackingTimesChartView;
    QGroupBox *groupBox_3;
    QWidget *gridLayoutWidget_4;
    QGridLayout *gridLayout_4;
    QLabel *cpuAverageFrameTimeLabel;
    QLabel *cpuMaxFrameTimeLabel;
    QLabel *cpuFramesProcessedLabel;
    QLabel *label_5;
    QLabel *label_4;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *cpuTotalProcessingTimeLabel;
    QGroupBox *groupBox_4;
    QWidget *gridLayoutWidget_5;
    QGridLayout *gridLayout_5;
    QLabel *label_9;
    QLabel *label_8;
    QLabel *label_10;
    QLabel *gpuAverageFrameTimeLabel;
    QLabel *gpuMaxFrameTimeLabel;
    QLabel *gpuFramesProcessedLabel;
    QLabel *label_6;
    QLabel *gpuTotalProcessingTimeLabel;

    void setupUi(QMainWindow *GuiClass)
    {
        if (GuiClass->objectName().isEmpty())
            GuiClass->setObjectName(QStringLiteral("GuiClass"));
        GuiClass->resize(1370, 840);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(GuiClass->sizePolicy().hasHeightForWidth());
        GuiClass->setSizePolicy(sizePolicy);
        GuiClass->setMinimumSize(QSize(1370, 840));
        GuiClass->setMaximumSize(QSize(1370, 840));
        centralWidget = new QWidget(GuiClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        tableView = new QTableView(centralWidget);
        tableView->setObjectName(QStringLiteral("tableView"));
        tableView->setGeometry(QRect(0, 0, 1371, 841));
        tableView->setMinimumSize(QSize(1371, 841));
        tableView->setMaximumSize(QSize(1371, 841));
        tableView->horizontalHeader()->setVisible(true);
        featureDetectionSettingsGroupBox = new QGroupBox(centralWidget);
        featureDetectionSettingsGroupBox->setObjectName(QStringLiteral("featureDetectionSettingsGroupBox"));
        featureDetectionSettingsGroupBox->setGeometry(QRect(20, 170, 281, 151));
        gridLayoutWidget_2 = new QWidget(featureDetectionSettingsGroupBox);
        gridLayoutWidget_2->setObjectName(QStringLiteral("gridLayoutWidget_2"));
        gridLayoutWidget_2->setGeometry(QRect(9, 29, 261, 111));
        gridLayout_2 = new QGridLayout(gridLayoutWidget_2);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        maxTrackedSpinBox = new QSpinBox(gridLayoutWidget_2);
        maxTrackedSpinBox->setObjectName(QStringLiteral("maxTrackedSpinBox"));
        maxTrackedSpinBox->setMinimum(10);
        maxTrackedSpinBox->setMaximum(1000);
        maxTrackedSpinBox->setSingleStep(10);
        maxTrackedSpinBox->setValue(200);

        gridLayout_2->addWidget(maxTrackedSpinBox, 0, 1, 1, 1);

        sensitivityLabel = new QLabel(gridLayoutWidget_2);
        sensitivityLabel->setObjectName(QStringLiteral("sensitivityLabel"));

        gridLayout_2->addWidget(sensitivityLabel, 1, 0, 1, 1);

        maxTrackedLabel = new QLabel(gridLayoutWidget_2);
        maxTrackedLabel->setObjectName(QStringLiteral("maxTrackedLabel"));

        gridLayout_2->addWidget(maxTrackedLabel, 0, 0, 1, 1);

        sensitivitySpinBox = new QDoubleSpinBox(gridLayoutWidget_2);
        sensitivitySpinBox->setObjectName(QStringLiteral("sensitivitySpinBox"));
        sensitivitySpinBox->setDecimals(3);
        sensitivitySpinBox->setMinimum(0.04);
        sensitivitySpinBox->setMaximum(0.06);
        sensitivitySpinBox->setSingleStep(0.001);
        sensitivitySpinBox->setValue(0.04);

        gridLayout_2->addWidget(sensitivitySpinBox, 1, 1, 1, 1);

        harrisThreshholdLabel = new QLabel(gridLayoutWidget_2);
        harrisThreshholdLabel->setObjectName(QStringLiteral("harrisThreshholdLabel"));

        gridLayout_2->addWidget(harrisThreshholdLabel, 2, 0, 1, 1);

        harrisThreshholdSpinBox = new QSpinBox(gridLayoutWidget_2);
        harrisThreshholdSpinBox->setObjectName(QStringLiteral("harrisThreshholdSpinBox"));
        harrisThreshholdSpinBox->setMinimum(10000);
        harrisThreshholdSpinBox->setMaximum(10000000);
        harrisThreshholdSpinBox->setSingleStep(10000);
        harrisThreshholdSpinBox->setValue(1000000);

        gridLayout_2->addWidget(harrisThreshholdSpinBox, 2, 1, 1, 1);

        progressGroupBox = new QGroupBox(centralWidget);
        progressGroupBox->setObjectName(QStringLiteral("progressGroupBox"));
        progressGroupBox->setGeometry(QRect(20, 580, 281, 151));
        cpuProgressBar = new QProgressBar(progressGroupBox);
        cpuProgressBar->setObjectName(QStringLiteral("cpuProgressBar"));
        cpuProgressBar->setGeometry(QRect(60, 40, 201, 23));
        cpuProgressBar->setValue(0);
        gpuProgressBar = new QProgressBar(progressGroupBox);
        gpuProgressBar->setObjectName(QStringLiteral("gpuProgressBar"));
        gpuProgressBar->setGeometry(QRect(60, 100, 201, 23));
        gpuProgressBar->setMaximum(100);
        gpuProgressBar->setValue(0);
        label_7 = new QLabel(progressGroupBox);
        label_7->setObjectName(QStringLiteral("label_7"));
        label_7->setGeometry(QRect(20, 40, 31, 16));
        label_11 = new QLabel(progressGroupBox);
        label_11->setObjectName(QStringLiteral("label_11"));
        label_11->setGeometry(QRect(20, 100, 31, 16));
        featureTrackingSettingsGroupBox = new QGroupBox(centralWidget);
        featureTrackingSettingsGroupBox->setObjectName(QStringLiteral("featureTrackingSettingsGroupBox"));
        featureTrackingSettingsGroupBox->setGeometry(QRect(20, 330, 281, 141));
        gridLayoutWidget = new QWidget(featureTrackingSettingsGroupBox);
        gridLayoutWidget->setObjectName(QStringLiteral("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(10, 30, 261, 101));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        correlationThreshholdSpinBox = new QDoubleSpinBox(gridLayoutWidget);
        correlationThreshholdSpinBox->setObjectName(QStringLiteral("correlationThreshholdSpinBox"));
        correlationThreshholdSpinBox->setDecimals(2);
        correlationThreshholdSpinBox->setMaximum(0.9);
        correlationThreshholdSpinBox->setSingleStep(0.01);
        correlationThreshholdSpinBox->setValue(0.5);

        gridLayout->addWidget(correlationThreshholdSpinBox, 0, 1, 1, 1);

        correlationThreshholdLabel = new QLabel(gridLayoutWidget);
        correlationThreshholdLabel->setObjectName(QStringLiteral("correlationThreshholdLabel"));

        gridLayout->addWidget(correlationThreshholdLabel, 0, 0, 1, 1);

        templateUpdateFramesSpinBox = new QSpinBox(gridLayoutWidget);
        templateUpdateFramesSpinBox->setObjectName(QStringLiteral("templateUpdateFramesSpinBox"));
        templateUpdateFramesSpinBox->setMinimum(1);
        templateUpdateFramesSpinBox->setMaximum(10);
        templateUpdateFramesSpinBox->setValue(3);

        gridLayout->addWidget(templateUpdateFramesSpinBox, 2, 1, 1, 1);

        templateUpdateFramesLabel = new QLabel(gridLayoutWidget);
        templateUpdateFramesLabel->setObjectName(QStringLiteral("templateUpdateFramesLabel"));

        gridLayout->addWidget(templateUpdateFramesLabel, 2, 0, 1, 1);

        label_12 = new QLabel(gridLayoutWidget);
        label_12->setObjectName(QStringLiteral("label_12"));

        gridLayout->addWidget(label_12, 3, 0, 1, 1);

        templateUpdateMaximumDistanceSpinBox = new QDoubleSpinBox(gridLayoutWidget);
        templateUpdateMaximumDistanceSpinBox->setObjectName(QStringLiteral("templateUpdateMaximumDistanceSpinBox"));
        templateUpdateMaximumDistanceSpinBox->setMinimum(1);
        templateUpdateMaximumDistanceSpinBox->setMaximum(25);
        templateUpdateMaximumDistanceSpinBox->setSingleStep(0.5);
        templateUpdateMaximumDistanceSpinBox->setValue(3.5);

        gridLayout->addWidget(templateUpdateMaximumDistanceSpinBox, 3, 1, 1, 1);

        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setGeometry(QRect(20, 480, 281, 91));
        startButton = new QPushButton(groupBox);
        startButton->setObjectName(QStringLiteral("startButton"));
        startButton->setEnabled(false);
        startButton->setGeometry(QRect(10, 30, 111, 51));
        stopButton = new QPushButton(groupBox);
        stopButton->setObjectName(QStringLiteral("stopButton"));
        stopButton->setEnabled(false);
        stopButton->setGeometry(QRect(160, 30, 111, 51));
        groupBox_2 = new QGroupBox(centralWidget);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        groupBox_2->setGeometry(QRect(20, 20, 281, 141));
        gridLayoutWidget_3 = new QWidget(groupBox_2);
        gridLayoutWidget_3->setObjectName(QStringLiteral("gridLayoutWidget_3"));
        gridLayoutWidget_3->setGeometry(QRect(9, 30, 261, 41));
        gridLayout_3 = new QGridLayout(gridLayoutWidget_3);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        gridLayout_3->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(gridLayoutWidget_3);
        label->setObjectName(QStringLiteral("label"));

        gridLayout_3->addWidget(label, 1, 0, 1, 1);

        maximumFramesSpinBox = new QSpinBox(gridLayoutWidget_3);
        maximumFramesSpinBox->setObjectName(QStringLiteral("maximumFramesSpinBox"));
        maximumFramesSpinBox->setMinimum(50);
        maximumFramesSpinBox->setMaximum(2500);
        maximumFramesSpinBox->setSingleStep(50);
        maximumFramesSpinBox->setValue(500);

        gridLayout_3->addWidget(maximumFramesSpinBox, 1, 1, 1, 1);

        openFlightFileButton = new QPushButton(groupBox_2);
        openFlightFileButton->setObjectName(QStringLiteral("openFlightFileButton"));
        openFlightFileButton->setGeometry(QRect(10, 80, 111, 51));
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tabWidget->setGeometry(QRect(320, 30, 1029, 796));
        imageDisplayTab = new QWidget();
        imageDisplayTab->setObjectName(QStringLiteral("imageDisplayTab"));
        imageDisplayLabel = new QLabel(imageDisplayTab);
        imageDisplayLabel->setObjectName(QStringLiteral("imageDisplayLabel"));
        imageDisplayLabel->setGeometry(QRect(0, 0, 1024, 768));
        tabWidget->addTab(imageDisplayTab, QString());
        statisticsTab = new QWidget();
        statisticsTab->setObjectName(QStringLiteral("statisticsTab"));
        trackingTimesChartView = new QtCharts::QChartView(statisticsTab);
        trackingTimesChartView->setObjectName(QStringLiteral("trackingTimesChartView"));
        trackingTimesChartView->setGeometry(QRect(20, 300, 981, 451));
        trackingTimesChartView->setFrameShadow(QFrame::Sunken);
        trackingTimesChartView->setInteractive(true);
        trackingTimesChartView->setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing);
        groupBox_3 = new QGroupBox(statisticsTab);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        groupBox_3->setGeometry(QRect(29, 20, 451, 251));
        gridLayoutWidget_4 = new QWidget(groupBox_3);
        gridLayoutWidget_4->setObjectName(QStringLiteral("gridLayoutWidget_4"));
        gridLayoutWidget_4->setGeometry(QRect(30, 40, 391, 191));
        gridLayout_4 = new QGridLayout(gridLayoutWidget_4);
        gridLayout_4->setSpacing(6);
        gridLayout_4->setContentsMargins(11, 11, 11, 11);
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        gridLayout_4->setContentsMargins(0, 0, 0, 0);
        cpuAverageFrameTimeLabel = new QLabel(gridLayoutWidget_4);
        cpuAverageFrameTimeLabel->setObjectName(QStringLiteral("cpuAverageFrameTimeLabel"));
        QFont font;
        font.setFamily(QStringLiteral("Lucida Console"));
        font.setPointSize(11);
        cpuAverageFrameTimeLabel->setFont(font);

        gridLayout_4->addWidget(cpuAverageFrameTimeLabel, 1, 1, 1, 1);

        cpuMaxFrameTimeLabel = new QLabel(gridLayoutWidget_4);
        cpuMaxFrameTimeLabel->setObjectName(QStringLiteral("cpuMaxFrameTimeLabel"));
        cpuMaxFrameTimeLabel->setFont(font);

        gridLayout_4->addWidget(cpuMaxFrameTimeLabel, 2, 1, 1, 1);

        cpuFramesProcessedLabel = new QLabel(gridLayoutWidget_4);
        cpuFramesProcessedLabel->setObjectName(QStringLiteral("cpuFramesProcessedLabel"));
        cpuFramesProcessedLabel->setFont(font);

        gridLayout_4->addWidget(cpuFramesProcessedLabel, 0, 1, 1, 1);

        label_5 = new QLabel(gridLayoutWidget_4);
        label_5->setObjectName(QStringLiteral("label_5"));
        QFont font1;
        font1.setFamily(QStringLiteral("Lucida Sans"));
        font1.setPointSize(11);
        label_5->setFont(font1);

        gridLayout_4->addWidget(label_5, 3, 0, 1, 1);

        label_4 = new QLabel(gridLayoutWidget_4);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setFont(font1);

        gridLayout_4->addWidget(label_4, 0, 0, 1, 1);

        label_2 = new QLabel(gridLayoutWidget_4);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setFont(font1);

        gridLayout_4->addWidget(label_2, 2, 0, 1, 1);

        label_3 = new QLabel(gridLayoutWidget_4);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setFont(font1);

        gridLayout_4->addWidget(label_3, 1, 0, 1, 1);

        cpuTotalProcessingTimeLabel = new QLabel(gridLayoutWidget_4);
        cpuTotalProcessingTimeLabel->setObjectName(QStringLiteral("cpuTotalProcessingTimeLabel"));
        cpuTotalProcessingTimeLabel->setFont(font);

        gridLayout_4->addWidget(cpuTotalProcessingTimeLabel, 3, 1, 1, 1);

        groupBox_4 = new QGroupBox(statisticsTab);
        groupBox_4->setObjectName(QStringLiteral("groupBox_4"));
        groupBox_4->setGeometry(QRect(530, 20, 451, 251));
        gridLayoutWidget_5 = new QWidget(groupBox_4);
        gridLayoutWidget_5->setObjectName(QStringLiteral("gridLayoutWidget_5"));
        gridLayoutWidget_5->setGeometry(QRect(30, 40, 391, 191));
        gridLayout_5 = new QGridLayout(gridLayoutWidget_5);
        gridLayout_5->setSpacing(6);
        gridLayout_5->setContentsMargins(11, 11, 11, 11);
        gridLayout_5->setObjectName(QStringLiteral("gridLayout_5"));
        gridLayout_5->setContentsMargins(0, 0, 0, 0);
        label_9 = new QLabel(gridLayoutWidget_5);
        label_9->setObjectName(QStringLiteral("label_9"));
        label_9->setFont(font1);

        gridLayout_5->addWidget(label_9, 1, 0, 1, 1);

        label_8 = new QLabel(gridLayoutWidget_5);
        label_8->setObjectName(QStringLiteral("label_8"));
        label_8->setFont(font1);

        gridLayout_5->addWidget(label_8, 2, 0, 1, 1);

        label_10 = new QLabel(gridLayoutWidget_5);
        label_10->setObjectName(QStringLiteral("label_10"));
        label_10->setFont(font1);

        gridLayout_5->addWidget(label_10, 0, 0, 1, 1);

        gpuAverageFrameTimeLabel = new QLabel(gridLayoutWidget_5);
        gpuAverageFrameTimeLabel->setObjectName(QStringLiteral("gpuAverageFrameTimeLabel"));
        gpuAverageFrameTimeLabel->setFont(font);

        gridLayout_5->addWidget(gpuAverageFrameTimeLabel, 1, 1, 1, 1);

        gpuMaxFrameTimeLabel = new QLabel(gridLayoutWidget_5);
        gpuMaxFrameTimeLabel->setObjectName(QStringLiteral("gpuMaxFrameTimeLabel"));
        gpuMaxFrameTimeLabel->setFont(font);

        gridLayout_5->addWidget(gpuMaxFrameTimeLabel, 2, 1, 1, 1);

        gpuFramesProcessedLabel = new QLabel(gridLayoutWidget_5);
        gpuFramesProcessedLabel->setObjectName(QStringLiteral("gpuFramesProcessedLabel"));
        gpuFramesProcessedLabel->setFont(font);

        gridLayout_5->addWidget(gpuFramesProcessedLabel, 0, 1, 1, 1);

        label_6 = new QLabel(gridLayoutWidget_5);
        label_6->setObjectName(QStringLiteral("label_6"));
        label_6->setFont(font1);

        gridLayout_5->addWidget(label_6, 3, 0, 1, 1);

        gpuTotalProcessingTimeLabel = new QLabel(gridLayoutWidget_5);
        gpuTotalProcessingTimeLabel->setObjectName(QStringLiteral("gpuTotalProcessingTimeLabel"));
        gpuTotalProcessingTimeLabel->setFont(font);

        gridLayout_5->addWidget(gpuTotalProcessingTimeLabel, 3, 1, 1, 1);

        tabWidget->addTab(statisticsTab, QString());
        GuiClass->setCentralWidget(centralWidget);

        retranslateUi(GuiClass);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(GuiClass);
    } // setupUi

    void retranslateUi(QMainWindow *GuiClass)
    {
        GuiClass->setWindowTitle(QApplication::translate("GuiClass", "Feature Tracking", Q_NULLPTR));
        featureDetectionSettingsGroupBox->setTitle(QApplication::translate("GuiClass", "Feature Detection Settings", Q_NULLPTR));
        sensitivityLabel->setText(QApplication::translate("GuiClass", "Detection sensitivity", Q_NULLPTR));
        maxTrackedLabel->setText(QApplication::translate("GuiClass", "Maximum tracked features", Q_NULLPTR));
        harrisThreshholdLabel->setText(QApplication::translate("GuiClass", "Harris response threshhold", Q_NULLPTR));
        progressGroupBox->setTitle(QApplication::translate("GuiClass", "Progress", Q_NULLPTR));
        label_7->setText(QApplication::translate("GuiClass", "CPU", Q_NULLPTR));
        label_11->setText(QApplication::translate("GuiClass", "GPU", Q_NULLPTR));
        featureTrackingSettingsGroupBox->setTitle(QApplication::translate("GuiClass", "Feature Tracking Settings", Q_NULLPTR));
        correlationThreshholdLabel->setText(QApplication::translate("GuiClass", "Correlation threshhold", Q_NULLPTR));
        templateUpdateFramesLabel->setText(QApplication::translate("GuiClass", "Template update frames", Q_NULLPTR));
        label_12->setText(QApplication::translate("GuiClass", "<html><head/><body><p>Template update<br/>maximum distance</p></body></html>", Q_NULLPTR));
        groupBox->setTitle(QApplication::translate("GuiClass", "Controls", Q_NULLPTR));
        startButton->setText(QApplication::translate("GuiClass", "Start", Q_NULLPTR));
        stopButton->setText(QApplication::translate("GuiClass", "Stop", Q_NULLPTR));
        groupBox_2->setTitle(QApplication::translate("GuiClass", "PANGU Settings", Q_NULLPTR));
        label->setText(QApplication::translate("GuiClass", "Maximum frames", Q_NULLPTR));
        openFlightFileButton->setText(QApplication::translate("GuiClass", "Open flight file...", Q_NULLPTR));
        imageDisplayLabel->setText(QString());
        tabWidget->setTabText(tabWidget->indexOf(imageDisplayTab), QApplication::translate("GuiClass", "Image Display", Q_NULLPTR));
        groupBox_3->setTitle(QApplication::translate("GuiClass", "CPU Stats", Q_NULLPTR));
        cpuAverageFrameTimeLabel->setText(QApplication::translate("GuiClass", "N/A", Q_NULLPTR));
        cpuMaxFrameTimeLabel->setText(QApplication::translate("GuiClass", "N/A", Q_NULLPTR));
        cpuFramesProcessedLabel->setText(QApplication::translate("GuiClass", "0", Q_NULLPTR));
        label_5->setText(QApplication::translate("GuiClass", "Total Processing Time", Q_NULLPTR));
        label_4->setText(QApplication::translate("GuiClass", "Frames Processed", Q_NULLPTR));
        label_2->setText(QApplication::translate("GuiClass", "Max Frame Time", Q_NULLPTR));
        label_3->setText(QApplication::translate("GuiClass", "Average Frame Time", Q_NULLPTR));
        cpuTotalProcessingTimeLabel->setText(QApplication::translate("GuiClass", "N/A", Q_NULLPTR));
        groupBox_4->setTitle(QApplication::translate("GuiClass", "GPU Stats", Q_NULLPTR));
        label_9->setText(QApplication::translate("GuiClass", "Average Frame Time", Q_NULLPTR));
        label_8->setText(QApplication::translate("GuiClass", "Max Frame Time", Q_NULLPTR));
        label_10->setText(QApplication::translate("GuiClass", "Frames Processed", Q_NULLPTR));
        gpuAverageFrameTimeLabel->setText(QApplication::translate("GuiClass", "N/A", Q_NULLPTR));
        gpuMaxFrameTimeLabel->setText(QApplication::translate("GuiClass", "N/A", Q_NULLPTR));
        gpuFramesProcessedLabel->setText(QApplication::translate("GuiClass", "0", Q_NULLPTR));
        label_6->setText(QApplication::translate("GuiClass", "Total Processing Time", Q_NULLPTR));
        gpuTotalProcessingTimeLabel->setText(QApplication::translate("GuiClass", "N/A", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(statisticsTab), QApplication::translate("GuiClass", "Statistics", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class GuiClass: public Ui_GuiClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GUI_H
