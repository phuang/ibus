/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et ts=4: */
/* ibus - The Input Bus
 * Copyright (C) 2008-2010 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2020 Takao Fujiwara <takao.fujiwara1@gmail.com>
 * Copyright (C) 2008-2020 Red Hat, Inc.
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

#include <gtk/gtk.h>
#include <gtk/gtkimmodule.h>
#include <ibus.h>
#include "ibusimcontext.h"

G_MODULE_EXPORT void
g_io_im_ibus_load (GTypeModule *type_module)
{
    static gboolean inited = FALSE;

    if (!inited) {
        ibus_init ();
        ibus_im_context_register_type (type_module);
        g_io_extension_point_implement ("gtk-im-module",
                                        IBUS_TYPE_IM_CONTEXT,
                                        "ibus",
                                        50);
        inited = TRUE;
    }
    /* make module resident */
    g_type_module_use (type_module);
}

G_MODULE_EXPORT void
g_io_im_ibus_unload (GTypeModule *type_module)
{
    g_type_module_unuse (type_module);
}

