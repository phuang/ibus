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
#ifndef __IBUS_CONFIG_PRIVATE_H_
#define __IBUS_CONFIG_PRIVATE_H_

static void
_from_dbus_value (IBusMessageIter   *iter,
                  GValue            *value)
{
    g_assert (iter != NULL);
    g_assert (value != NULL);

    GType type;
    IBusMessageIter sub_iter;

    type = ibus_message_iter_get_arg_type (iter);
    if (type == IBUS_TYPE_VARIANT) {
        ibus_message_iter_recurse (iter, IBUS_TYPE_VARIANT, &sub_iter);
        iter = &sub_iter;
        type = ibus_message_iter_get_arg_type (iter);
    }

    switch (type) {
    case G_TYPE_STRING:
        {
            gchar *v;
            g_value_init (value, G_TYPE_STRING);
            ibus_message_iter_get_basic (iter, &v);
            g_value_set_string (value, v);
        }
        break;
    case G_TYPE_INT:
        {
            gint v;
            g_value_init (value, G_TYPE_INT);
            ibus_message_iter_get_basic (iter, &v);
            g_value_set_int (value, v);
        }
        break;
    case G_TYPE_UINT:
        {
            guint v;
            g_value_init (value, G_TYPE_UINT);
            ibus_message_iter_get_basic (iter, &v);
            g_value_set_uint (value, v);
        }
        break;
    case G_TYPE_BOOLEAN:
        {
            gboolean v;
            g_value_init (value, G_TYPE_BOOLEAN);
            ibus_message_iter_get_basic (iter, &v);
            g_value_set_boolean (value, v);
        }
        break;
    case G_TYPE_DOUBLE:
        {
            gdouble v;
            g_value_init (value, G_TYPE_DOUBLE);
            ibus_message_iter_get_basic (iter, &v);
            g_value_set_double (value, v);
        }
        break;
    default:
        if (type == IBUS_TYPE_ARRAY) {
            GValue v = { 0 };
            IBusMessageIter sub_iter;
            GType sub_type;
            GValueArray *array;


            sub_type = ibus_message_iter_get_element_type (iter);
            g_assert (sub_type == G_TYPE_STRING ||
                      sub_type == G_TYPE_INT ||
                      sub_type == G_TYPE_UINT ||
                      sub_type == G_TYPE_BOOLEAN ||
                      sub_type == G_TYPE_DOUBLE ||
                      sub_type == IBUS_TYPE_VARIANT);

            g_value_init (value, G_TYPE_VALUE_ARRAY);
            array = g_value_array_new (0);
            ibus_message_iter_recurse (iter, IBUS_TYPE_ARRAY, &sub_iter);
            while (ibus_message_iter_get_arg_type (&sub_iter) != G_TYPE_INVALID) {
                _from_dbus_value (&sub_iter, &v);
                ibus_message_iter_next (&sub_iter);
                g_value_array_append (array, &v);
                g_value_unset (&v);
            }
            g_value_take_boxed (value, array);
            break;
        }

        g_debug ("type=%s", g_type_name (type));

        g_assert_not_reached ();
    }
}

static void
_to_dbus_value (IBusMessageIter *iter,
                const GValue    *value)
{
    IBusMessageIter sub_iter;
    gboolean retval;


    switch (G_VALUE_TYPE (value)) {
    case G_TYPE_STRING:
        {
            retval = ibus_message_iter_open_container (iter, IBUS_TYPE_VARIANT, "s", &sub_iter);
            g_assert (retval);

            const gchar *v = g_value_get_string (value);
            ibus_message_iter_append (&sub_iter,
                                      G_TYPE_STRING,
                                      &v);
            ibus_message_iter_close_container (iter, &sub_iter);
        }
        break;
    case G_TYPE_INT:
        {
            retval = ibus_message_iter_open_container (iter, IBUS_TYPE_VARIANT, "i", &sub_iter);
            g_assert (retval);

            gint v = g_value_get_int (value);
            ibus_message_iter_append (&sub_iter,
                                      G_TYPE_INT,
                                      &v);
            ibus_message_iter_close_container (iter, &sub_iter);
        }
        break;
    case G_TYPE_UINT:
        {
            retval = ibus_message_iter_open_container (iter, IBUS_TYPE_VARIANT, "u", &sub_iter);
            g_assert (retval);

            guint v = g_value_get_uint (value);
            ibus_message_iter_append (&sub_iter,
                                      G_TYPE_UINT,
                                      &v);
            ibus_message_iter_close_container (iter, &sub_iter);
        }
        break;
    case G_TYPE_BOOLEAN:
        {
            retval = ibus_message_iter_open_container (iter, IBUS_TYPE_VARIANT, "b", &sub_iter);
            g_assert (retval);

            gboolean v = g_value_get_boolean (value);
            ibus_message_iter_append (&sub_iter,
                                      G_TYPE_BOOLEAN,
                                      &v);
            ibus_message_iter_close_container (iter, &sub_iter);
        }
        break;
    case G_TYPE_DOUBLE:
        {
            retval = ibus_message_iter_open_container (iter, IBUS_TYPE_VARIANT, "d", &sub_iter);
            g_assert (retval);

            gdouble v = g_value_get_double (value);
            ibus_message_iter_append (&sub_iter,
                                      G_TYPE_DOUBLE,
                                      &v);
            ibus_message_iter_close_container (iter, &sub_iter);
        }
        break;
    default:
        if (G_TYPE_VALUE_ARRAY == G_VALUE_TYPE (value)) {
            IBusMessageIter sub_sub_iter;
            GType type = G_TYPE_INVALID;
            gint i;

            retval = ibus_message_iter_open_container (iter, IBUS_TYPE_VARIANT, "av", &sub_iter);
            g_assert (retval);

            GValueArray *array = (GValueArray *)g_value_get_boxed (value);
            ibus_message_iter_open_container (&sub_iter,
                                              IBUS_TYPE_ARRAY,
                                              "v",
                                              &sub_sub_iter);
            if (array->n_values > 0) {
                type = G_VALUE_TYPE (&array->values[0]);
                g_assert (type == G_TYPE_STRING ||
                          type == G_TYPE_INT ||
                          type == G_TYPE_UINT ||
                          type == G_TYPE_BOOLEAN ||
                          type == G_TYPE_DOUBLE);
            }
            for (i = 0; i < array->n_values; i++) {
                g_assert (type == G_VALUE_TYPE (&array->values[i]));
                _to_dbus_value (&sub_sub_iter, &array->values[i]);
            }
            ibus_message_iter_close_container (&sub_iter, &sub_sub_iter);
            ibus_message_iter_close_container (iter, &sub_iter);
            break;
        }
        g_assert_not_reached();
    }

}
#endif

