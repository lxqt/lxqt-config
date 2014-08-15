#include "main.h"
#include <QApplication>

#include "monitorsettingsdialog.h"
#include "xrandr.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    XRandRBackend *xrandr = new XRandRBackend();
    MonitorSettingsDialog dlg(xrandr);
    dlg.show();

    return app.exec();
}
