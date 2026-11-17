/*
    This file is part of the LXQt project.

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

#ifndef _WLR_BACKEND_H_
#define _WLR_BACKEND_H_

#include "kscreenutils.h"

#include <KScreen/Config>

#include <QString>

#include <functional>

/** Wayland compositors with wlr-output-management (labwc, sway, wayfire, ...), which libkscreen doesn't support.
 * KScreen is only used for its data classes here.
 */
class WlrBackend {
public:
    /** Whether the compositor offers wlr-output-management.
     */
    static bool isAvailable();

    /** Reads the outputs asynchronously. The callback gets a null config if the compositor didn't send them.
     */
    static void readConfig(std::function<void(KScreen::ConfigPtr)> callback);

    /** Asks the compositor whether config would work, without applying it.
     */
    static bool canApply(const KScreen::ConfigPtr &config);

    static bool applyConfig(const KScreen::ConfigPtr &config);

    /** The protocol has no EDID, only make, model and serial number. Empty if there is no such output.
     */
    static KScreenUtils::Identity identity(const QString &outputName);
};

#endif // _WLR_BACKEND_H_
