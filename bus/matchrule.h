/* vim:set et sts=4: */
/* IBus - The Input Bus
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
#ifndef __MATCH_RULE_H_
#define __MATCH_RULE_H_

#include <glib-object.h>
typedef struct _BusMatchRule BusMatchRule;

struct _BusMatchRule {
    gint   refcount;
    gint   message_type;
    gchar *interface;
    gchar *member;
    gchar *sender;
    gchar *destination;
    gchar *path;

    gchar **args;
    gint args_len;
};

G_BEGIN_DECLS

BusMatchRule    *bus_match_rule_new         (const gchar    *text);
void             bus_match_rule_unref       (BusMatchRule   *rule);
gboolean         bus_match_rule_set_message_type
                                            (BusMatchRule   *rule,
                                             gint            type);
gboolean         bus_match_rule_set_sender  (BusMatchRule   *rule,
                                             const gchar    *sender);
gboolean         bus_match_rule_set_interface
                                            (BusMatchRule   *rule,
                                             const gchar    *interface);
gboolean         bus_match_rule_set_member  (BusMatchRule   *rule,
                                             const gchar    *member);
gboolean         bus_match_rule_set_path    (BusMatchRule   *rule,
                                             const gchar    *path);
gboolean         bus_match_rule_set_destination
                                            (BusMatchRule   *rule,
                                             const gchar    *dest);
gboolean         bus_match_rule_set_arg     (BusMatchRule   *rule,
                                             guint           arg_index,
                                             const gchar    *arg);
gboolean         bus_match_rule_match       (BusMatchRule   *rule,
                                             DBusMessage    *message);

G_END_DECLS
#endif

