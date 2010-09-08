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
 * SECTION: ibusdebug
 * @short_description: Debug message output.
 * @stability: Stable
 *
 * This section lists functions that generate debug and warning messages.
 */
#ifndef __IBUS_DEBUG_H_
#define __IBUS_DEBUG_H_

/**
 * ibus_warning:
 * @msg: A printf formatted message to be print.
 * @args...: Necessary arguments for @msg.
 *
 * A convenient wrapper for g_warning.
 * The output format will be
 * <programlisting>
 *     source_file:line, message...
 * </programlisting>
 */
#define ibus_warning(msg, args...) \
    g_warning("%s:%d, " msg, __FILE__, __LINE__, ##args)

#endif

