/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* ibus
 * Copyright (C) 2008 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2008 Red Hat, Inc.
 *
 * gdk-private.h:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */
#ifndef __GTK_PRIVATE_H_
#define __GTK_PRIVATE_H_

void
translate_key_event (GdkDisplay *display,
		     GdkEvent   *event,
		     XEvent     *xevent);
#endif
