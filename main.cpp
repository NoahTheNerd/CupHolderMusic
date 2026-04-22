// Some testing stuff I made, leaving it for reference for various things
// #include <QApplication>
// #include <QPushButton>
// #include <QLabel>
// #include <QGridLayout>
//
// int main(int argc, char *argv[]) {
//     QApplication a(argc, argv);
//     QWidget window;
//     QGridLayout *layout = new QGridLayout(&window);
//     QLabel label("Hello world!", nullptr);
//     QPushButton button("Hello world.. BUTTON!", nullptr);
//     layout->addWidget(&button, 0, 0);
//     layout->addWidget(&label, 1, 0, Qt::AlignCenter);
//
//     window.show();
//     return QApplication::exec();
// }

#include <QApplication>
#include "mainwindow.h"
#include "database.h"

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	a.setOrganizationName("CupHolderMusic");
	a.setApplicationName("CupHolderMusic");

	Database::instance().open();

	MainWindow w;
	w.show();
	w.showMaximized();

	return QApplication::exec();
}
