#include "tvsegwindow.h"
#include <QApplication>

#include "tvseg/settings/backend.h"
#include "tvseg/settings/serializerqt.h"
#include "tvseg/util/logging.h"


_INITIALIZE_EASYLOGGINGPP


int main(int argc, char *argv[])
{
    // Setup debugging
    _START_EASYLOGGINGPP(argc, argv);
    easyloggingpp::Configurations confFromFile("tvseg.el.cfg");
    easyloggingpp::Loggers::reconfigureAllLoggers(confFromFile);

    // Launch application
    QApplication a(argc, argv);
    tvseg_ui::TVSegWindow w(NULL, "tvseg.ini");
    w.initFromSettings();
    w.show();

    int result = a.exec();

    return result;
}
