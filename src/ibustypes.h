/* vim:set et ts=4: */
/* ibus - The Input Bus
 * Copyright (C) 2008-2009 Huang Peng <shawn.p.huang@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef __IBUS_TYPES_H_
#define __IBUS_TYPES_H_

typedef enum
{
    GDK_SHIFT_MASK    = 1 << 0,
    GDK_LOCK_MASK     = 1 << 1,
    GDK_CONTROL_MASK  = 1 << 2,
    GDK_MOD1_MASK     = 1 << 3,
    GDK_MOD2_MASK     = 1 << 4,
    GDK_MOD3_MASK     = 1 << 5,
    GDK_MOD4_MASK     = 1 << 6,
    GDK_MOD5_MASK     = 1 << 7,
    GDK_BUTTON1_MASK  = 1 << 8,
    GDK_BUTTON2_MASK  = 1 << 9,
    GDK_BUTTON3_MASK  = 1 << 10,
    GDK_BUTTON4_MASK  = 1 << 11,
    GDK_BUTTON5_MASK  = 1 << 12,
  
    /* The next few modifiers are used by XKB, so we skip to the end.
     * Bits 15 - 25 are currently unused. Bit 29 is used internally.
     */
    
    GDK_SUPER_MASK    = 1 << 26,
    GDK_HYPER_MASK    = 1 << 27,
    GDK_META_MASK     = 1 << 28,
    
    GDK_RELEASE_MASK  = 1 << 30,
  
    GDK_MODIFIER_MASK = 0x5c001fff
} GdkModifierType;

#endif

