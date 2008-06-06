/* vim:set et ts=4: */
/* GIK - The G Input Toolkit
 * Copyright (C) 2008-2009 Huang Peng
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
#include <config.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <string.h>
#include <stdarg.h>
#include <glib/gstdio.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#ifdef HAVE_SYS_INOTIFY_H
#define HAVE_INOTIFY
#  include <sys/inotify.h>
#endif
#include "gikimclient.h"

#define IBUS_NAME  "org.freedesktop.IBus"
#define IBUS_IFACE "org.freedesktop.IBus"
#define IBUS_PATH  "/org/freedesktop/IBus"
#define IBUS_ADDR "unix:abstract=/tmp/ibus"
//#define IBUS_ADDR "tcp:host=localhost,port=7799"

/* GikIMClientPriv */
struct _GikIMClientPrivate {
#if USE_DBUS_SESSION_BUS
    DBusConnection  *dbus;
#endif

#ifdef HAVE_INOTIFY
    /* inotify */
    gint            inotify_wd;
    GIOChannel      *inotify_channel;
    guint           inotify_source;
#endif

    DBusConnection  *ibus;
    gboolean         enable;

    GtkIMContext    *context;

    /* preedit status */
    gchar           *preedit_string;
    PangoAttrList   *preedit_attrs;
    gint             preedit_cursor;
    gboolean         preedit_show;
};

/* functions prototype */
static void     gik_im_client_class_init   (GikIMClientClass    *klass);
static void     gik_im_client_init         (GikIMClient         *client);
static void     gik_im_client_finalize     (GObject             *obj);

static void     gik_im_client_commit_string(GikIMClient         *client,
                                            const gchar         *string);
static void     gik_im_client_update_preedit
                                           (GikIMClient         *client,
                                            const gchar         *string,
                                            PangoAttrList       *attrs,
                                            gint                cursor_pos,
                                            gboolean            show);

static void     gik_im_client_sync_hotkeys (GikIMClient         *client);
static gboolean _ibus_call_with_reply_and_block
                                           (DBusConnection      *connection,
                                            const gchar         *method,
                                            int                 first_arg_type,
                                                                ...);
static gboolean _ibus_call_with_reply      (DBusConnection      *connection,
                                            const gchar         *method,
                                            DBusPendingCallNotifyFunction
                                                                function,
                                            void                *data,
                                            DBusFreeFunction    free_function,
                                            int                 first_arg_type,
                                                                ...);
static gboolean _dbus_call_with_reply_and_block
                                           (DBusConnection      *connection,
                                            const gchar         *dest,
                                            const gchar         *path,
                                            const gchar         *iface,
                                            const char          *method,
                                            gint                first_arg_type,
                                                                ...);

/* callback functions */
static DBusHandlerResult
                _gik_im_client_message_filter_cb
                                            (DBusConnection      *connection,
                                             DBusMessage         *message,
                                             void                *user_data);

static void     _dbus_name_owner_changed_cb (DBusGProxy          *proxy,
                                             const gchar         *name,
                                             const gchar         *old_name,
                                             const gchar         *new_name,
                                             GikIMClient         *client);

static GType                gik_type_im_client = 0;
static GtkObjectClass       *parent_class = NULL;
static GikIMClient          *_client = NULL;
static gboolean             has_focus = FALSE;


GType
gik_im_client_get_type (void)
{
    g_assert (gik_type_im_client != 0);
    return gik_type_im_client;
}

void
gik_im_client_register_type (GTypeModule *type_module)
{
    static const GTypeInfo gik_im_client_info = {
        sizeof (GikIMClientClass),
        (GBaseInitFunc)        NULL,
        (GBaseFinalizeFunc)     NULL,
        (GClassInitFunc)     gik_im_client_class_init,
        NULL,            /* class finialize */
        NULL,            /* class data */
        sizeof (GikIMClient),
        0,
        (GInstanceInitFunc)    gik_im_client_init,
    };

    if (! gik_type_im_client ) {
        gik_type_im_client =
            g_type_module_register_type (type_module,
                GTK_TYPE_OBJECT,
                "GikIMClient",
                &gik_im_client_info,
                (GTypeFlags)0);
    }
}


GikIMClient *
gik_im_client_get_client (void)
{
    if (_client == NULL) {
        _client = GIK_IM_CLIENT(g_object_new (GIK_TYPE_IM_CLIENT, NULL));
        g_object_ref_sink (_client);
    }
    else {
        g_object_ref (_client);
    }

    return _client;
}


static void
gik_im_client_class_init     (GikIMClientClass *klass)
{
    GtkObjectClass *object_class = GTK_OBJECT_CLASS (klass);
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    parent_class = (GtkObjectClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (GikIMClientPrivate));

    gobject_class->finalize = &gik_im_client_finalize;
}

#if 0
static void
_gik_im_client_reinit_imm (GikIMClient *client)
{
    GError *error;
    GikIMClientPrivate *priv = client->priv;

    if (priv->imm != NULL) {
        g_object_unref (priv->imm);
    }

    /* get gik proxy */
    error = NULL;
    priv->imm = dbus_g_proxy_new_for_name_owner (priv->ibus,
                                GIK_DBUS_SERVICE,
                                GIK_DBUS_PATH,
                                GIK_DBUS_INTERFACE,
                                &error);

    if (priv->imm == NULL) {
        g_warning (error->message);
        g_error_free (error);
        return;
    }

    error = NULL;
    if (!dbus_g_proxy_call (priv->imm, "register_client", &error,
                            G_TYPE_INVALID, G_TYPE_INVALID)) {
        g_warning ("%s", error->message);
        g_error_free (error);
        g_object_unref (priv->imm);
        priv->imm = NULL;
        return;
    }

    gik_im_client_sync_hotkeys (client);
    g_debug ("new imm %s", dbus_g_proxy_get_bus_name (priv->imm));
}
#endif

static void
_gik_im_client_ibus_open (GikIMClient *client)
{
    gchar *ibus_addr = NULL;
    DBusError error;

    GikIMClientPrivate *priv = client->priv;

    if (priv->ibus != NULL)
        return;

#if USE_DBUS_SESSION_BUS
    dbus_connection_setup_with_g_main (priv->dbus, NULL);
    if (!_dbus_call_with_reply_and_block (priv->dbus,
                        IBUS_NAME, IBUS_PATH, IBUS_IFACE,
                        "GetIBusAddress",
                        DBUS_TYPE_INVALID,
                        DBUS_TYPE_STRING, &ibus_addr,
                        DBUS_TYPE_INVALID
                        )) {
        g_warning ("Can not get ibus address");
        return;
    }
#endif
    if (ibus_addr == NULL) {
        gchar *display, *host, *id;
        display = g_strdup (g_getenv ("DISPLAY"));
        if (display != NULL) {
            id = host = display;
            for (; *id != ':' && *id != '\0'; id++);
            if (*id == '\0')
                id = "";
            else {
                *id = '\0';
                id ++;
            }
        }
        else {
            host = id = "";
        }
        ibus_addr = g_strdup_printf ("unix:path=/tmp/ibus-%s/ibus-%s-%s", g_get_user_name (), host, id);
        g_free (display);
    }

    /*
     * Init ibus and proxy object
     */
    dbus_error_init (&error);
    priv->ibus = dbus_connection_open_private (ibus_addr, &error);
    g_free (ibus_addr);
    if (priv->ibus == NULL) {
        g_warning ("Error: %s", error.message);
        dbus_error_free (&error);
        return;
    }

    if (!dbus_connection_add_filter (priv->ibus,
            _gik_im_client_message_filter_cb,
            client, NULL)) {
        g_warning ("Out of memory");
        return;
    }
    dbus_connection_setup_with_g_main (priv->ibus, NULL);
    const gchar *app_name = g_get_application_name ();
    _ibus_call_with_reply_and_block (priv->ibus, "RegisterClient",
                DBUS_TYPE_STRING, &app_name,
                DBUS_TYPE_INVALID,
                DBUS_TYPE_INVALID);

}

static void
_gik_im_client_ibus_close (GikIMClient *client)
{
    DBusError error;

    GikIMClientPrivate *priv = client->priv;

    if (priv->ibus) {
        dbus_connection_close (priv->ibus);
        dbus_connection_unref (priv->ibus);
        priv->ibus = NULL;
    }

    if (priv->preedit_string) {
        g_free (priv->preedit_string);
        priv->preedit_string = NULL;
    }

    if (priv->preedit_attrs) {
        pango_attr_list_unref (priv->preedit_attrs);
        priv->preedit_attrs = NULL;
    }

    if (priv->context) {
        g_signal_emit_by_name (priv->context, "preedit-changed");
    }

    priv->enable = FALSE;
}

#ifdef HAVE_INOTIFY
static gboolean
_gik_im_client_inotify_cb (GIOChannel *source, GIOCondition condition, GikIMClient *client)
{
    struct inotify_event *p = NULL;
    gchar *name;
    gsize n;

    if (condition & G_IO_IN == 0)
        return TRUE;

    p = g_malloc0 (sizeof (struct inotify_event) + 1024);

    g_io_channel_read_chars (source, (gchar *) p, sizeof (struct inotify_event),  &n, NULL);
    g_io_channel_read_chars (source, ((gchar *)p) + sizeof (struct inotify_event), p->len,  &n, NULL);

    name = g_strdup_printf ("ibus-%s", g_getenv ("DISPLAY"));
    for (n = 0; name[n] != 0; n++) {
        if (name[n] != ':')
            continue;
        name[n] = '-';
        break;
    }

    if (g_strcmp0 (p->name, name) == 0) {
        if (p->mask & IN_CREATE) {
            g_usleep (1000);
            _gik_im_client_ibus_open (client);
        }
    }
    g_free (name);
    g_free (p);

}
#endif

static void
gik_im_client_init (GikIMClient *obj)
{
    DEBUG_FUNCTION_IN;

    DBusError error;
    GikIMClient *client = GIK_IM_CLIENT (obj);
    GikIMClientPrivate *priv;

    gchar *watch_path;
    struct stat stat_buf;

#ifdef HAVE_INOTIFY
    gint inotify_fd = inotify_init ();
#endif

    priv = G_TYPE_INSTANCE_GET_PRIVATE (client, GIK_TYPE_IM_CLIENT, GikIMClientPrivate);
    client->priv = priv;

    priv->context = NULL;

    priv->preedit_string = NULL;
    priv->preedit_attrs = NULL;
    priv->preedit_cursor = 0;
    priv->enable = FALSE;

    watch_path = g_strdup_printf ("/tmp/ibus-%s", g_get_user_name ());

    if (g_stat (watch_path, &stat_buf) != 0) {
        g_mkdir (watch_path, 0750);
    }

#ifdef HAVE_INOTIFY
    /* init inotify */
    priv->inotify_wd = inotify_add_watch (inotify_fd, watch_path, IN_CREATE | IN_DELETE);
    priv->inotify_channel = g_io_channel_unix_new (inotify_fd);
    g_io_channel_set_close_on_unref (priv->inotify_channel, TRUE);
    priv->inotify_source = g_io_add_watch (priv->inotify_channel,
                                    G_IO_IN,
                                    (GIOFunc)_gik_im_client_inotify_cb,
                                    (gpointer)client);
#endif
    g_free (watch_path);

#if USE_DBUS_SESSION_BUS
    /*
     * Init dbus
     */
    dbus_error_init (&error);
    priv->dbus = dbus_bus_get (DBUS_BUS_SESSION, &error);
    if (priv->dbus == NULL) {
        g_warning ("Error: %s", error.message);
        dbus_error_free (&error);
        return;
    }
#endif

    _gik_im_client_ibus_open (client);

#if USE_DBUS_SESSION_BUS
    if (!dbus_connection_add_filter (priv->dbus,
            _gik_im_client_message_filter_cb,
            client, NULL)) {
        g_warning ("Out of memory");
        return;
    }

    gchar *rule =
            "type='signal',"
            "sender='" DBUS_SERVICE_DBUS "',"
            "interface='" DBUS_INTERFACE_DBUS "',"
            "member='NameOwnerChanged',"
            "path='" DBUS_PATH_DBUS "',"
            "arg0='" IBUS_NAME "'";

    if (!_dbus_call_with_reply_and_block (priv->dbus,
                        DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS,
                        "AddMatch",
                        DBUS_TYPE_STRING, &rule,
                        DBUS_TYPE_INVALID,
                        DBUS_TYPE_INVALID
                        )) {
        g_warning ("Can not get ibus address");
        return;
    }
#endif
#if 0
    /* get dbus proxy */
    priv->dbus = dbus_g_proxy_new_for_name (priv->ibus,
                                DBUS_SERVICE_DBUS,
                                DBUS_PATH_DBUS,
                                DBUS_INTERFACE_DBUS);
    g_assert (priv->dbus != NULL);

    /* connect NameOwnerChanged signal */
    dbus_g_proxy_add_signal (priv->dbus, "NameOwnerChanged",
                                G_TYPE_STRING,
                                G_TYPE_STRING,
                                G_TYPE_STRING,
                                G_TYPE_INVALID);

    dbus_g_proxy_connect_signal (priv->dbus, "NameOwnerChanged",
                                G_CALLBACK (_dbus_name_owner_changed_cb),
                                (gpointer)client, NULL);
    dbus_bus_add_match ((DBusConnection *)dbus_g_connection_get_connection (priv->ibus),
                        "type='signal',"
                        "sender='" DBUS_SERVICE_DBUS
                        "',interface='" DBUS_INTERFACE_DBUS
                        "',path='" DBUS_PATH_DBUS
                        "',member='NameOwnerChanged',"
                        "arg0='" GIK_DBUS_SERVICE "'",
                        &dbus_error);

     _gik_im_client_reinit_imm (client);
#endif

}


static void
gik_im_client_finalize (GObject *obj)
{
    DEBUG_FUNCTION_IN;

    GikIMClient *client = GIK_IM_CLIENT (obj);
    GikIMClientPrivate *priv = client->priv;

    g_assert (client == _client);

#ifdef HAVE_INOTIFY
    g_source_remove (priv->inotify_source);
    g_io_channel_unref (priv->inotify_channel);
#endif

#if USE_DBUS_SESSION_BUS
    if (priv->dbus) {
        dbus_connection_unref (priv->dbus);
    }
#endif
    _gik_im_client_ibus_close (client);

    G_OBJECT_CLASS(parent_class)->finalize (obj);

    _client = NULL;
}


void
gik_im_client_set_im_context (GikIMClient *client, GtkIMContext *context)
{
    GikIMClientPrivate *priv = client->priv;
    priv->context = context;
}

GtkIMContext *
gik_im_client_get_im_context (GikIMClient *client)
{
    GikIMClientPrivate *priv = client->priv;
    return priv->context;
}

static void
gik_im_client_commit_string (GikIMClient *client, const gchar *string)
{
    GikIMClientPrivate *priv = client->priv;

    if (priv->context) {
        g_signal_emit_by_name (priv->context, "commit", string);
    }
}

static void
gik_im_client_update_preedit (GikIMClient *client, const gchar *string,
        PangoAttrList *attrs, gint cursor_pos, gboolean show)
{
    GikIMClientPrivate *priv = client->priv;
    if (priv->preedit_string) {
        g_free (priv->preedit_string);
    }
    priv->preedit_string =  g_strdup (string);

    if (priv->preedit_attrs) {
        pango_attr_list_unref (priv->preedit_attrs);
    }

    priv->preedit_attrs = attrs;
    if (attrs) {
        pango_attr_list_ref (priv->preedit_attrs);
    }

    priv->preedit_cursor = cursor_pos;
    priv->preedit_show = show;
    if (priv->context) {
        g_signal_emit_by_name (priv->context, "preedit-changed");
    }
}

static void
_gik_signal_commit_string_handler (DBusConnection *connection, DBusMessage *message, GikIMClient *client)
{
    /* Handle CommitString signal */
    DBusError error = {0};
    gchar *string = NULL;

    if (!dbus_message_get_args (message, &error,
            DBUS_TYPE_STRING, &string, DBUS_TYPE_INVALID)) {
        g_warning ("%s", error.message);
        dbus_error_free (&error);
    }
    else {
        gik_im_client_commit_string (client, string);
    }
}

static void
_gik_signal_update_preedit_handler (DBusConnection *connection, DBusMessage *message, GikIMClient *client)
{
    /* Handle UpdatePreedit signal */
    DBusError error = {0};
    DBusMessageIter iter, sub_iter;
    gint type, sub_type;

    gchar *string = NULL;
    PangoAttrList *attrs = NULL;
    gint cursor = 0;
    gboolean show = False;

    if (!dbus_message_iter_init (message, &iter)) {
        g_warning ("The UpdatePreedit signal does have args!");
        return;
    }

    type = dbus_message_iter_get_arg_type (&iter);
    if (type != DBUS_TYPE_STRING) {
        g_warning ("The frist argument of UpdatePreedit signal must be a String");
        return;
    }
    dbus_message_iter_get_basic (&iter, &string);
    dbus_message_iter_next (&iter);


    type = dbus_message_iter_get_arg_type (&iter);
    if (type != DBUS_TYPE_ARRAY) {
        g_warning ("The secode argument of UpdatePreedit signal must be a Struct Array");
        return;
    }

    dbus_message_iter_recurse (&iter, &sub_iter);

    if (dbus_message_iter_get_arg_type (&sub_iter) != DBUS_TYPE_INVALID) {
        if (dbus_message_iter_get_arg_type (&sub_iter) != DBUS_TYPE_ARRAY ||
            dbus_message_iter_get_element_type (&sub_iter) != DBUS_TYPE_INT32 ) {
            g_warning ("The secode argument of UpdatePreedit signal must be a Struct Array");
            return;
        }

        attrs = pango_attr_list_new ();

        while ((sub_type = dbus_message_iter_get_arg_type (&sub_iter) != DBUS_TYPE_INVALID)) {
            PangoAttribute *attr;
            DBusMessageIter sub_sub_iter;
            guint *values = NULL;
            gint length = 0;
            dbus_message_iter_recurse (&sub_iter, &sub_sub_iter);
            dbus_message_iter_get_fixed_array (&sub_sub_iter, &values, &length);

            if (length <= 0) {
                g_warning ("The element of the second argument of UpdatePreedit should not be a empty array");
                continue;
            }

            switch (values[0]) {
            case 1: /* Underline */
                attr = pango_attr_underline_new (values[1]);
                attr->start_index = values[2];
                attr->end_index = values[3];
                pango_attr_list_insert (attrs, attr);
                break;

            case 2: /* Foreground Color */
                attr = pango_attr_foreground_new (
                                (values[1] & 0xff0000) >> 8,
                                (values[1] & 0x00ff00),
                                (values[1] & 0x0000ff) << 8
                                );
                attr->start_index = values[2];
                attr->end_index = values[3];
                pango_attr_list_insert (attrs, attr);
                break;
            case 3: /* Background Color */
                attr = pango_attr_background_new (
                                (values[1] & 0xff0000) >> 8,
                                (values[1] & 0x00ff00),
                                (values[1] & 0x0000ff) << 8
                                );
                attr->start_index = values[2];
                attr->end_index = values[3];
                pango_attr_list_insert (attrs, attr);
                break;
            default:
                g_warning ("Unkown type attribute type = %d", values[0]);

            }

            dbus_message_iter_next (&sub_iter);

        }
    }
    dbus_message_iter_next (&iter);

    type = dbus_message_iter_get_arg_type (&iter);
    if (type != DBUS_TYPE_INT32) {
        g_warning ("The third argument of UpdatePreedit signal must be an Int32 %c", type);
        pango_attr_list_unref (attrs);
        return;
    }
    dbus_message_iter_get_basic (&iter, &cursor);
    dbus_message_iter_next (&iter);

    type = dbus_message_iter_get_arg_type (&iter);
    if (type != DBUS_TYPE_BOOLEAN) {
        g_warning ("The third argument of UpdatePreedit signal must be an Int32 %c", type);
        pango_attr_list_unref (attrs);
        return;
    }
    dbus_message_iter_get_basic (&iter, &show);
    dbus_message_iter_next (&iter);

    gik_im_client_update_preedit (client, string, attrs, cursor, show);
    pango_attr_list_unref (attrs);

}

#ifdef USE_DBUS_SESSION_BUS
static void
_gik_signal_name_owner_changed_handler (DBusConnection *connection, DBusMessage *message, GikIMClient *client)
{
    GikIMClientPrivate *priv = client->priv;

    gchar *name = NULL;
    gchar *old_name = NULL;
    gchar *new_name = NULL;
    DBusError error = {0};

    if (!dbus_message_get_args (message, &error,
            DBUS_TYPE_STRING, &name,
            DBUS_TYPE_STRING, &old_name,
            DBUS_TYPE_STRING, &new_name,
            DBUS_TYPE_INVALID)) {
        g_warning ("%s", error.message);
        dbus_error_free (&error);
    }

    g_return_if_fail (strcmp (name, IBUS_NAME) == 0);

    if (g_strcmp0 (new_name, "") == 0) {
        _gik_im_client_ibus_close (client);
        priv->enable = FALSE;
    }
    else {
        _gik_im_client_ibus_open (client);
        priv->enable = TRUE;
    }
}
#endif

static void
_gik_signal_disconnected_handler (DBusConnection *connection, DBusMessage *message, GikIMClient *client)
{
    _gik_im_client_ibus_close (client);
}

static void
_gik_signal_enabled_handler (DBusConnection *connection, DBusMessage *message, GikIMClient *client)
{
    DEBUG_FUNCTION_IN;
    client->priv->enable = TRUE;
}


static void
_gik_signal_disabled_handler (DBusConnection *connection, DBusMessage *message, GikIMClient *client)
{
    DEBUG_FUNCTION_IN;
    client->priv->enable = FALSE;
}
static DBusHandlerResult
_gik_im_client_message_filter_cb (DBusConnection *connection, DBusMessage *message, void *user_data)
{
    GikIMClient *client = (GikIMClient *) user_data;

    static struct SIGNAL_HANDLER {
        const gchar *iface;
        const gchar *name;
        void (* handler) (DBusConnection *, DBusMessage *, GikIMClient *);
    } handlers[] = {
#ifdef USE_DBUS_SESSION_BUS
        { DBUS_INTERFACE_DBUS, "NameOwnerChanged", _gik_signal_name_owner_changed_handler },
#endif
        { DBUS_INTERFACE_LOCAL, "Disconnected", _gik_signal_disconnected_handler },
        { IBUS_IFACE, "CommitString", _gik_signal_commit_string_handler },
        { IBUS_IFACE, "UpdatePreedit", _gik_signal_update_preedit_handler },
        { IBUS_IFACE, "Enabled", _gik_signal_enabled_handler },
        { IBUS_IFACE, "Disabled", _gik_signal_disabled_handler },
        {0},
    };

    gint i;
    for (i = 0; handlers[i].iface != NULL; i++) {
        if (dbus_message_is_signal (message, handlers[i].iface, handlers[i].name)) {
            handlers[i].handler (connection, message, client);
            return DBUS_HANDLER_RESULT_HANDLED;
        }
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

inline static gboolean
_dbus_call_with_reply_and_block_valist (DBusConnection *connection,
    const gchar *dest, const gchar *path, const gchar* iface, const char *method,
    gint first_arg_type, va_list args)
{
    DBusMessage *message, *reply;
    DBusError error = {0};
    int type;

    if (connection == NULL)
        return FALSE;

    message = dbus_message_new_method_call (dest,
                                    path, iface, method);
    if (!message) {
        g_warning ("Out of memory!");
        return FALSE;
    }

    if (!dbus_message_append_args_valist (message, first_arg_type, args)) {
        dbus_message_unref (message);
        g_warning ("Can not create call message");
        return FALSE;
    }

    reply = dbus_connection_send_with_reply_and_block (connection,
                        message, -1, &error);

    dbus_message_unref (message);

    if (!reply) {
        g_warning ("%s", error.message);
        dbus_error_free (&error);
        return FALSE;
    }

    type = first_arg_type;
    while (type != DBUS_TYPE_INVALID) {
        if (type == DBUS_TYPE_ARRAY) {
            va_arg (args, int);
            va_arg (args, void *);
            va_arg (args, int);
        }
        else {
            va_arg (args, int);
        }
        type = va_arg (args, int);
    }

    type = va_arg (args, int);
    if (!dbus_message_get_args_valist (reply, &error, type, args)) {
        g_warning ("%s", error.message);
        dbus_error_free (&error);
        dbus_message_unref (reply);
        return FALSE;
    }
    dbus_message_unref (reply);

    return TRUE;

}

inline static gboolean
_dbus_call_with_reply_and_block (DBusConnection *connection,
    const gchar *dest, const gchar *path, const gchar* iface, const char *method,
    gint first_arg_type, ...)
{
    va_list args;
    gboolean retval;

    if (connection == NULL)
        return FALSE;

    va_start (args, first_arg_type);
    retval = _dbus_call_with_reply_and_block_valist (connection,
                    dest, path, iface, method, first_arg_type, args);
    va_end (args);

    return TRUE;

}

static gboolean
_ibus_call_with_reply_and_block (DBusConnection *connection, const gchar *method, int first_arg_type, ...)
{
    va_list args;
    gboolean retval;

    if (connection == NULL)
        return FALSE;

    va_start (args, first_arg_type);
    retval = _dbus_call_with_reply_and_block_valist (connection,
                    IBUS_NAME, IBUS_PATH, IBUS_IFACE, method, first_arg_type, args);
    va_end (args);

    return retval;

}


inline static gboolean
_dbus_call_with_reply_valist (DBusConnection *connection,
    const gchar *dest, const gchar *path, const gchar* iface, const char *method,
    DBusPendingCallNotifyFunction notify_function,
    void *user_data, DBusFreeFunction free_function,
    gint first_arg_type, va_list args)
{
    DBusMessage *message = NULL;
    DBusPendingCall *pendingcall = NULL;
    DBusError error = {0};
    int type;

    if (connection == NULL) {
        goto error;
    }

    message = dbus_message_new_method_call (dest,
                                    path, iface, method);
    if (!message) {
        g_warning ("Out of memory!");
        goto error;
    }

    if (!dbus_message_append_args_valist (message, first_arg_type, args)) {
        g_warning ("Can not create call message");
        goto error;
    }

    if (!dbus_connection_send_with_reply (connection,
                        message, &pendingcall, -1)) {
        g_warning ("Out of memory!");
        goto error;
    }

    if (!dbus_pending_call_set_notify (pendingcall, notify_function,
            user_data, free_function)) {
        g_warning ("Out of memory!");
        goto error;
    }

    dbus_message_unref (message);
    return TRUE;

error:
    if (message)
        dbus_message_unref (message);
    if (pendingcall)
        dbus_pending_call_cancel (pendingcall);
    if (user_data && free_function)
        free_function (user_data);
    return False;
}

inline static gboolean
_dbus_call_with_reply (DBusConnection *connection,
    const gchar *dest, const gchar *path, const gchar* iface, const char *method,
    DBusPendingCallNotifyFunction notify_function,
    void *user_data, DBusFreeFunction free_function,
    gint first_arg_type, ...)
{
    va_list args;
    gboolean retval;

    if (connection == NULL)
        return FALSE;

    va_start (args, first_arg_type);
    retval = _dbus_call_with_reply_valist (connection,
                    dest, path, iface, method,
                    notify_function,
                    user_data, free_function,
                    first_arg_type, args);
    va_end (args);

    return TRUE;

}



static gboolean
_ibus_call_with_reply (DBusConnection *connection, const gchar *method,
       DBusPendingCallNotifyFunction notify_function,
       void *user_data, DBusFreeFunction free_function,
       int first_arg_type, ...)
{
    va_list args;
    gboolean retval;

    if (connection == NULL)
        return FALSE;

    va_start (args, first_arg_type);
    retval = _dbus_call_with_reply_valist (connection,
                    IBUS_NAME, IBUS_PATH, IBUS_IFACE,
                    method, notify_function,
                    user_data, free_function,
                    first_arg_type, args);
    va_end (args);

    return retval;
}


static void
_gik_filter_keypress_reply_cb (DBusPendingCall *pending, void *user_data)
{
    DBusMessage *reply;
    DBusError error = {0};
    GdkEvent *event = (GdkEvent *) user_data;
    gboolean retval;


    reply = dbus_pending_call_steal_reply (pending);
    dbus_pending_call_unref (pending);

    if (dbus_set_error_from_message (&error, reply)) {
        g_warning ("%s", error.message);
        dbus_error_free (&error);
        retval = FALSE;
    }
    else {
        if (!dbus_message_get_args (reply, &error,
                DBUS_TYPE_BOOLEAN, &retval, DBUS_TYPE_INVALID)) {
            g_warning ("%s", error.message);
            dbus_error_free (&error);
            retval = FALSE;
        }
    }

    if (!retval) {
        event->any.send_event = TRUE;
        gdk_event_put (event);
    }
}

gboolean
gik_im_client_filter_keypress (GikIMClient *client, GdkEventKey *event)
{
    GikIMClientPrivate *priv = client->priv;

    guint state = event->state & GDK_MODIFIER_MASK;
    gboolean is_press = event->type == GDK_KEY_PRESS;

    if (event->send_event) {
        return FALSE;
    }

    /* Call IBus ProcessKeyEvent method */
    if (!_ibus_call_with_reply (priv->ibus,
            "ProcessKeyEvent",
            _gik_filter_keypress_reply_cb,
            gdk_event_copy ((GdkEvent *)event),
            (DBusFreeFunction)gdk_event_free,
            DBUS_TYPE_UINT32, &event->keyval,
            DBUS_TYPE_BOOLEAN, &is_press,
            DBUS_TYPE_UINT32, &state,
            DBUS_TYPE_INVALID))
        return FALSE;

    return TRUE;
}


void
gik_im_client_focus_in (GikIMClient *client)
{
    /* Call IBus FocusIn method */
     _ibus_call_with_reply_and_block (client->priv->ibus,
            "FocusIn",
            DBUS_TYPE_INVALID,
            DBUS_TYPE_INVALID);
}

void
gik_im_client_focus_out (GikIMClient *client)
{
    /* Call IBus FocusOut method */
    _ibus_call_with_reply_and_block (client->priv->ibus,
            "FocusOut",
            DBUS_TYPE_INVALID,
            DBUS_TYPE_INVALID);

}

void
gik_im_client_reset (GikIMClient *client)
{
    /* Call IBus Reset method */
    _ibus_call_with_reply_and_block (client->priv->ibus,
            "Reset",
            DBUS_TYPE_INVALID,
            DBUS_TYPE_INVALID);

}


gboolean
gik_im_client_get_preedit_string (
    GikIMClient *client,
    gchar         **str,
    PangoAttrList **attrs,
    gint           *cursor_pos
)
{
    GikIMClientPrivate *priv = client->priv;

    if (!priv->preedit_show) {
        if (str) *str = g_strdup ("");
        if (attrs) *attrs = pango_attr_list_new ();
        if (cursor_pos) *cursor_pos = 0;
        return TRUE;
    }

    if (str) {
        *str = g_strdup (priv->preedit_string ? priv->preedit_string: "");
    }

    if (attrs) {
        if (priv->preedit_attrs) {
            *attrs = pango_attr_list_ref (priv->preedit_attrs);
        }
        else {
            *attrs = pango_attr_list_new ();
        }
    }

    if (cursor_pos) {
        *cursor_pos = priv->preedit_cursor;
    }

    return TRUE;
}


void
gik_im_client_set_client_window  (GikIMClient *client, GdkWindow *window)
{
    GikIMClientPrivate *priv = client->priv;
}

void
gik_im_client_set_cursor_location (GikIMClient *client, GdkRectangle *area)
{
    GikIMClientPrivate *priv = client->priv;

    _ibus_call_with_reply_and_block (client->priv->ibus,
            "SetCursorLocation",
            DBUS_TYPE_INT32, &area->x,
            DBUS_TYPE_INT32, &area->y,
            DBUS_TYPE_INT32, &area->width,
            DBUS_TYPE_INT32, &area->height,
            DBUS_TYPE_INVALID,
            DBUS_TYPE_INVALID);
}


gboolean
gik_im_client_is_enabled (GikIMClient *client)
{
    return (client->priv->ibus != NULL) && (client->priv->enable);
}

static void
gik_im_client_sync_hotkeys (GikIMClient *client)
{
    GError *error;
    gchar **hotkeys = NULL;
    gint i;

    GikIMClientPrivate *priv = client->priv;
#if 0
    g_return_if_fail (priv->imm != NULL);

    error = NULL;
    if (!dbus_g_proxy_call (priv->imm, "get_hotkeys", &error,
                            G_TYPE_INVALID,
                            G_TYPE_STRV, &hotkeys,
                            G_TYPE_INVALID)) {
        if (error) {
            g_warning ("%s", error->message);
            g_error_free (error);
        }
        return;
    }

    for (i = 0; i < g_strv_length (hotkeys); i++) {
        g_debug ("hotkeys[%d] = %s", i, hotkeys[i]);
    }
    g_strfreev (hotkeys);
#endif

}

/* Callback functions for slave context */
#if 0
static void
_dbus_name_owner_changed_cb (
    DBusGProxy *proxy,
    const gchar *name,
    const gchar *prev_owner,
    const gchar *new_owner,
    GikIMClient *client)
{
    GikIMClientPrivate *priv = client->priv;

    if (strcmp (name, GIK_DBUS_SERVICE) == 0) {
        if (strcmp (new_owner, "") == 0) {
            /* Gik service exited */
            if (priv->imm) {
                g_object_unref (priv->imm);
                priv->imm = NULL;
            }
            if (priv->ime) {
                g_object_unref (priv->ime);
                priv->ime = NULL;
            }
        }
        else {
            /* Gik service avaliable or owner changed */
            _gik_im_client_reinit_imm (client);
        }
    }
}

#endif
