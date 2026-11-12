/*
    Copyright (C) 2019  P.L. Lucas <selairi@gmail.com>

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

#ifndef _KSCREEN_UTILS_H_
#define _KSCREEN_UTILS_H_

#include <KScreen/Config>
#include <KScreen/Output>

#include <QString>

#include <functional>

class KScreenUtils {
public:
    /** Reads the current configuration asynchronously.
     * The callback gets a null config if no backend could provide one.
     */
    static void getConfig(std::function<void(KScreen::ConfigPtr)> callback);

    /** Applies config and waits until it is done. Returns false if it failed.
     */
    static bool setConfig(const KScreen::ConfigPtr &config);

    /** Checks whether config is valid for the current monitors.
     */
    static bool canApply(const KScreen::ConfigPtr &config);

    /** Virtual screen size is updated.
     * If all monitors are allocated over a big rectangle, the size of this rectangle is the virtual screen size.
     */
    static void updateScreenSize(KScreen::ConfigPtr &config);
    
    /** This method applies config and it shows a TimeoutDialog to undo the changes.
     */
    static bool applyConfig(KScreen::ConfigPtr &config, KScreen::ConfigPtr &oldConfig);
    
    /** Sets the monitors aligned side by side.
     */
    static void extended(KScreen::ConfigPtr &config);

    /** Width of output in the global layout, after rotation and scale. 0 without a mode.
     */
    static int logicalWidth(const KScreen::OutputPtr &output);

    /** Whether the backend knows a primary monitor. wlr-output-management doesn't.
     */
    static bool supportsPrimary();

    /** Whether monitors can show the same picture. wlr-output-management can't.
     */
    static bool supportsUnified();

    /** Whether each monitor can have its own scale, which only Wayland does.
     */
    static bool supportsScale();

    /** Makes setEnabled(false) work on outputs that a backend creates itself.
     */
    static void allowDisabling(const KScreen::OutputPtr &output);

    struct Identity {
        QString vendor;
        QString name;
        QString serial;
    };

    /** The monitor connected to output, from its EDID, or from what the backend has instead.
     */
    static Identity identity(const KScreen::OutputPtr &output);

    /** Identifies the monitor in saved settings. Empty if unknown.
     */
    static QString monitorHash(const KScreen::OutputPtr &output);
};

#endif // _KSCREEN_UTILS_H_
