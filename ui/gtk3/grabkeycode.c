/* vim:set et sts=4 sw=4:
 *
 * ibus - The Input Bus
 *
 * Copyright(c) 2011 Peng Huang <shawn.p.huang@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or(at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 */

#include <glib.h>
#include <gdk/gdkx.h>
#include <X11/extensions/XInput2.h>

gboolean grab_keycode (GdkDisplay *display,
                       guint keyval,
                       guint modifiers) {
    Display *xdisplay = GDK_DISPLAY_XDISPLAY (display);
    int keycode = XKeysymToKeycode (xdisplay, keyval);
    if (keycode == 0) {
        g_warning ("Can not convert keyval=%u to keycode!", keyval);
        return FALSE;
    }

    XIEventMask mask;
    mask.deviceid = XIAllMasterDevices;
    mask.mask_len = XIMaskLen(XI_RawMotion);
    mask.mask = g_new0 (unsigned char, mask.mask_len);
    XISetMask (mask.mask, XI_KeyPress);
    XISetMask (mask.mask, XI_KeyRelease);

    XIGrabModifiers ximodifiers[] = {
        { modifiers, 0 },
        { LockMask | modifiers, 0 },
        { Mod2Mask | modifiers, 0 },
        { Mod5Mask | modifiers, 0 },
        { LockMask | Mod2Mask | modifiers, 0 },
        { Mod2Mask | Mod5Mask | modifiers, 0 },
        { LockMask | Mod5Mask | modifiers, 0 },
        { LockMask | Mod2Mask | Mod5Mask | modifiers, 0 },
    };

    int retval = XIGrabKeycode (xdisplay,
                                XIAllMasterDevices,
                                keycode,
                                DefaultRootWindow (xdisplay),
                                GrabModeAsync,
                                GrabModeAsync,
                                True,
                                &mask,
                                G_N_ELEMENTS (ximodifiers),
                                ximodifiers);

    g_free (mask.mask);

    if (retval == -1)
        return FALSE;
    return TRUE;
}

gboolean ungrab_keycode (GdkDisplay *display,
                         guint keyval,
                         guint modifiers) {
    Display *xdisplay = GDK_DISPLAY_XDISPLAY (display);
    int keycode = XKeysymToKeycode (xdisplay, keyval);
    if (keycode == 0) {
        g_warning ("Can not convert keyval=%u to keycode!", keyval);
        return FALSE;
    }

    XIGrabModifiers ximodifiers[] = {
        {modifiers, 0},
        {Mod2Mask | modifiers, 0},
        {LockMask | modifiers, 0},
        {Mod5Mask | modifiers, 0},
        {Mod2Mask | LockMask | modifiers, 0},
        {Mod2Mask | Mod5Mask | modifiers, 0},
        {LockMask | Mod5Mask | modifiers, 0},
        {Mod2Mask | LockMask | Mod5Mask | modifiers, 0},
    };

    int retval = XIUngrabKeycode (xdisplay,
                                  XIAllMasterDevices,
                                  keycode,
                                  DefaultRootWindow (xdisplay),
                                  G_N_ELEMENTS (ximodifiers),
                                  ximodifiers);

    if (retval == -1)
        return FALSE;

    return TRUE;
}
