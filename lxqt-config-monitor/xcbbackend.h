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

#ifndef _XCB_BACKEND_H_
#define _XCB_BACKEND_H_

#include <KScreen/Config>

/** Native X11 access through RandR, as libkscreen dropped X11 in 6.8.
 * KScreen is only used for its data classes here.
 */
class XcbBackend {
public:
    /** Returns a null config if RandR 1.3 or later is not available.
     */
    static KScreen::ConfigPtr readConfig();

    /** Checks whether the enabled monitors of config can be driven by the CRTCs of the graphics card.
     */
    static bool canApply(const KScreen::ConfigPtr &config);

    static bool applyConfig(const KScreen::ConfigPtr &config);
};

#endif // _XCB_BACKEND_H_
