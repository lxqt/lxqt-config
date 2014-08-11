#include "lxrandr-qt.h"
#include <QApplication>

#include "monitorsettingsdialog.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    
    MonitorSettingsDialog dlg;
    dlg.show();

    return app.exec();
}
