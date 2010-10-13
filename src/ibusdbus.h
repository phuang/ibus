/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/**
 * SECTION: ibusdbus
 * @Title: IBusDBus
 * @Short_description: DBus types
 * @Stability: Stable
 *
 */
#ifndef __IBUS_DBUS_H_
#define __IBUS_DBUS_H_

G_BEGIN_DECLS

#ifndef DBUS_H
typedef struct IBusError IBusError;
typedef struct IBusMessage IBusMessage;
typedef struct IBusMessageIter IBusMessageIter;
typedef struct IBusPendingCall IBusPendingCall;
typedef struct DBusServer DBusServer;
typedef struct DBusConnection DBusConnection;
#else
/**
 * IBusError:
 *
 * A data type representing an IBusError.
 * An IBusError is actually a #DBusError.
 *
 * @see_also: #DBusError for detail structure definition.
 */
typedef DBusError IBusError;

/**
 * IBusMessage:
 *
 * An opaque data structure that represents IBusMessage.
 */
typedef DBusMessage IBusMessage;

/**
 * IBusMessageIter:
 *
 * An opaque data structure that represents IBusMessageIter.
 */
typedef DBusMessageIter IBusMessageIter;

/**
 * IBusPendingCall:
 *
 * An opaque data structure that represents IBusPendingCall.
 */
typedef DBusPendingCall IBusPendingCall;

#endif

G_END_DECLS

#endif
