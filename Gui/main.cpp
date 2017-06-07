#include <QtWidgets/QApplication>
#include <qstylefactory.h>

#include "gui.h"

void init_theme();

int main(int argc, char **argv) {
    QApplication application(argc, argv);
	application.setWindowIcon(QIcon("./Resources/moon.ico"));
    Gui window;
	init_theme();
    window.show();
    return application.exec();
}

/* Theme colour settings from https://gist.github.com/QuantumCD/6245215 */
void init_theme() {
	qApp->setStyle(QStyleFactory::create("Fusion"));

	QPalette dark_palette;
	dark_palette.setColor(QPalette::Window, QColor(53, 53, 53));
	dark_palette.setColor(QPalette::WindowText, Qt::white);
	dark_palette.setColor(QPalette::Base, QColor(25, 25, 25));
	dark_palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
	dark_palette.setColor(QPalette::ToolTipBase, Qt::white);
	dark_palette.setColor(QPalette::ToolTipText, Qt::white);
	dark_palette.setColor(QPalette::Text, Qt::white);
	dark_palette.setColor(QPalette::Button, QColor(53, 53, 53));
	dark_palette.setColor(QPalette::ButtonText, Qt::white);
	dark_palette.setColor(QPalette::BrightText, Qt::red);
	dark_palette.setColor(QPalette::Link, QColor(42, 130, 218));
	dark_palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
	dark_palette.setColor(QPalette::HighlightedText, Qt::black);

	qApp->setPalette(dark_palette);
	qApp->setStyleSheet(
		"QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }"
		":disabled { color: #444444; background-color: #161616; }"
	);
}
