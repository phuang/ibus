/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* ibus - The Input Bus
 * Copyright (c) 2010, Google Inc. All rights reserved.
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

#ifndef MEMCONF_CONFIG_H_
#define MEMCONF_CONFIG_H_

#include <map>
#include <string>

#include <ibus.h>

struct IBusConfigMemConf {
  IBusConfigService parent;
  // We have to use pointer type here for |entries| since g_object_new() uses
  // malloc rather than new to create IBusConfigMemConf object.
  std::map<std::string, GValue*>* entries;
};

IBusConfigMemConf* ibus_config_memconf_new(IBusConnection* connection);

// These tiny hacks are necessary since memconf/main.cc which is copied
// from gconf/main.c on compile-time uses "gconf" rather than "memconf."
typedef IBusConfigMemConf IBusConfigGConf;
#define ibus_config_gconf_new ibus_config_memconf_new

#endif  // MEMCONF_CONFIG_H_
