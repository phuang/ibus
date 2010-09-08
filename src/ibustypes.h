/* vim:set et sts=4: */
/* ibus - The Input Bus
 * Copyright (C) 2008-2010 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2008-2010 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.         See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/**
 * SECTION: ibustypes
 * @short_description: Generic types for  IBus.
 * @stability: Stable
 *
 * This section consists generic types for IBus, including shift/control key modifiers,
 * and a rectangle structure.
 */
#ifndef __IBUS_TYPES_H_
#define __IBUS_TYPES_H_

/**
 * IBusModifierType:
 * @IBUS_SHIFT_MASK: Shift  is activated.
 * @IBUS_LOCK_MASK: Cap Lock is locked.
 * @IBUS_CONTROL_MASK: Control key is activated.
 * @IBUS_MOD1_MASK: Modifier 1 (Usually Alt_L (0x40),  Alt_R (0x6c),  Meta_L (0xcd)) activated.
 * @IBUS_MOD2_MASK: Modifier 2 (Usually Num_Lock (0x4d)) activated.
 * @IBUS_MOD3_MASK: Modifier 3 activated.
 * @IBUS_MOD4_MASK: Modifier 4 (Usually Super_L (0xce),  Hyper_L (0xcf)) activated.
 * @IBUS_MOD5_MASK: Modifier 5 (ISO_Level3_Shift (0x5c),  Mode_switch (0xcb)) activated.
 * @IBUS_BUTTON1_MASK: Mouse button 1 (left) is activated.
 * @IBUS_BUTTON2_MASK: Mouse button 2 (middle) is activated.
 * @IBUS_BUTTON3_MASK: Mouse button 3 (right) is activated.
 * @IBUS_BUTTON4_MASK: Mouse button 4 (scroll up) is activated.
 * @IBUS_BUTTON5_MASK: Mouse button 5 (scroll down) is activated.
 * @IBUS_HANDLED_MASK: Handled mask indicates the event has been handled by ibus.
 * @IBUS_FORWARD_MASK: Forward mask indicates the event has been forward from ibus.
 * @IBUS_IGNORED_MASK: It is an alias of IBUS_FORWARD_MASK.
 * @IBUS_SUPER_MASK: Super (Usually Win) key is activated.
 * @IBUS_HYPER_MASK: Hyper key is activated.
 * @IBUS_META_MASK: Meta key is activated.
 * @IBUS_RELEASE_MASK: Key is released.
 * @IBUS_MODIFIER_MASK: Modifier mask for the all the masks above.
 *
 * Handles key modifier such as control, shift and alt and release event.
 * Note that nits 15 - 25 are currently unused, while bit 29 is used internally.
 */
typedef enum
{
    IBUS_SHIFT_MASK    = 1 << 0,
    IBUS_LOCK_MASK     = 1 << 1,
    IBUS_CONTROL_MASK  = 1 << 2,
    IBUS_MOD1_MASK     = 1 << 3,
    IBUS_MOD2_MASK     = 1 << 4,
    IBUS_MOD3_MASK     = 1 << 5,
    IBUS_MOD4_MASK     = 1 << 6,
    IBUS_MOD5_MASK     = 1 << 7,
    IBUS_BUTTON1_MASK  = 1 << 8,
    IBUS_BUTTON2_MASK  = 1 << 9,
    IBUS_BUTTON3_MASK  = 1 << 10,
    IBUS_BUTTON4_MASK  = 1 << 11,
    IBUS_BUTTON5_MASK  = 1 << 12,

    /* The next few modifiers are used by XKB, so we skip to the end.
     * Bits 15 - 23 are currently unused. Bit 29 is used internally.
     */

    /* ibus mask */
    IBUS_HANDLED_MASK  = 1 << 24,
    IBUS_FORWARD_MASK  = 1 << 25,
    IBUS_IGNORED_MASK  = IBUS_FORWARD_MASK,

    IBUS_SUPER_MASK    = 1 << 26,
    IBUS_HYPER_MASK    = 1 << 27,
    IBUS_META_MASK     = 1 << 28,

    IBUS_RELEASE_MASK  = 1 << 30,

    IBUS_MODIFIER_MASK = 0x5f001fff
} IBusModifierType;

/**
 * IBusCapabilite:
 * @IBUS_CAP_PREEDIT_TEXT: UI is capable to show pre-edit text.
 * @IBUS_CAP_AUXILIARY_TEXT: UI is capable to show auxiliary text.
 * @IBUS_CAP_LOOKUP_TABLE: UI is capable to show the lookup table.
 * @IBUS_CAP_FOCUS: UI is capable to get focus.
 * @IBUS_CAP_PROPERTY: UI is capable to have property.
 * @IBUS_CAP_SURROUNDING_TEXT: Client can provide surround text,
 *  or IME can handle surround text.
 *
 * Capability flags of UI.
 */
typedef enum {
    IBUS_CAP_PREEDIT_TEXT       = 1 << 0,
    IBUS_CAP_AUXILIARY_TEXT     = 1 << 1,
    IBUS_CAP_LOOKUP_TABLE       = 1 << 2,
    IBUS_CAP_FOCUS              = 1 << 3,
    IBUS_CAP_PROPERTY           = 1 << 4,
    IBUS_CAP_SURROUNDING_TEXT   = 1 << 5,
} IBusCapabilite;

/**
 * IBusPreeditFocusMode:
 * @IBUS_ENGINE_PREEDIT_CLEAR: pre-edit text is cleared.
 * @IBUS_ENGINE_PREEDIT_COMMIT: pre-edit text is committed.
 *
 * Pre-edit commit mode when the focus is lost.
 */
typedef enum {
    IBUS_ENGINE_PREEDIT_CLEAR   = 0,
    IBUS_ENGINE_PREEDIT_COMMIT  = 1,
} IBusPreeditFocusMode;

/**
 * IBusOrientation:
 * @IBUS_ORIENTATION_HORIZONTAL: Horizontal orientation.
 * @IBUS_ORIENTATION_VERTICAL: Vertival orientation.
 * @IBUS_ORIENTATION_SYSTEM: Use ibus global orientation setup.
 *
 * Orientation of UI.
 */
typedef enum {
    IBUS_ORIENTATION_HORIZONTAL = 0,
    IBUS_ORIENTATION_VERTICAL   = 1,
    IBUS_ORIENTATION_SYSTEM     = 2,
} IBusOrientation;

/**
 * IBusRectangle:
 * @x: x coordinate.
 * @y: y coordinate.
 * @width: width of the rectangle.
 * @height: height of the renctangl.
 *
 * Rectangle definition.
 */
typedef struct _IBusRectangle IBusRectangle;
struct _IBusRectangle {
    gint x;
    gint y;
    gint width;
    gint height;
};

/**
 * IBusFreeFunc:
 * @object: object to be freed.
 *
 * Free function prototype.
 */
typedef void (* IBusFreeFunc) (gpointer object);

#endif

