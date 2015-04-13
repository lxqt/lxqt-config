/*
    Copyright (C) 2014  P.L. Lucas <selairi@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "main.h"
#include <LXQt/SingleApplication>
#include <LXQt/ConfigDialog>
#include <LXQt/Settings>
#include "monitorsettingsdialog.h"
#include "quickoptions.h"
#include "xrandr.h"
#include "applydialog.h"

int main(int argc, char** argv) {
	LxQt::SingleApplication app(argc, argv);
	
	QByteArray configName = qgetenv("LXQT_SESSION_CONFIG");
	if(configName.isEmpty())
		configName = "session";
	LxQt::Settings settings(configName);
	LxQt::ConfigDialog dlg(QObject::tr("Monitor Settings"), &settings);
	app.setActivationWindow(&dlg);
	dlg.setWindowIcon(QIcon::fromTheme("preferences-desktop-display"));
	
	XRandRBackend *xrandr = new XRandRBackend();
	MonitorSettingsDialog *monitorSettingsDialog = new MonitorSettingsDialog(xrandr);
	
	{
		QList<MonitorInfo*> monitorsInfo = xrandr->getMonitorsInfo();
		// If this is a laptop and there is an external monitor, offer quick options
		if(monitorsInfo.length() == 2) {
			QuickOptions *quickOptions = new QuickOptions();
			monitorSettingsDialog->connect(quickOptions->ui.useBoth, SIGNAL(clicked(bool)), SLOT(onUseBoth()));
			monitorSettingsDialog->connect(quickOptions->ui.externalOnly, SIGNAL(clicked(bool)), SLOT(onExternalOnly()));
			monitorSettingsDialog->connect(quickOptions->ui.laptopOnly, SIGNAL(clicked(bool)), SLOT(onLaptopOnly()));
			monitorSettingsDialog->connect(quickOptions->ui.extended, SIGNAL(clicked(bool)), SLOT(onExtended()));
			dlg.addPage(quickOptions, QObject::tr("Quick Options"), "format-justify-left");
		}
	 }
	
	dlg.addPage(monitorSettingsDialog, QObject::tr("Settings"), "preferences-desktop-display");
	
	ApplyDialog *apply = new ApplyDialog();
	monitorSettingsDialog->connect(apply->ui.apply, SIGNAL(clicked(bool)), SLOT(applySettings()));
	monitorSettingsDialog->connect(apply->ui.save, SIGNAL(clicked(bool)), SLOT(saveSettings()));
	
	
	dlg.addPage(apply, QObject::tr("Apply"), "system-run");

	dlg.exec();
	
	return 0;
	}
