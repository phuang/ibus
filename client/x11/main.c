/* vim:set et sts=4: */
/* ibus
 * Copyright (C) 2007-2008 Huang Peng <shawn.p.huang@gmail.com>
 *
 * main.c:
 *
 * This tool is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <XimProto.h>
#include <IMdkit.h>
#include <Xi18n.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <langinfo.h>
#include <locale.h>
#include <iconv.h>
#include <signal.h>
#include <stdlib.h>

#define _GNU_SOURCES
#include <getopt.h>

#define LOG(level, fmt, args...) \
    if (g_debug_level >= (level)) { \
        g_debug (fmt, args); \
    }

#include <ibusattribute.h>
#include <ibusimclient.h>
#include "gdk-private.h"

struct _X11ICONN {
    GList        *clients;
};
typedef struct _X11ICONN    X11ICONN;

struct _X11IC {
    gchar       *ibus_ic;
    Window      client_window;
    Window      focus_window;
    gint32      input_style;
    X11ICONN    *conn;
    gint        icid;
    gint        connect_id;
    gchar       *lang;
    gboolean        has_preedit_area;
    GdkRectangle    preedit_area;

    gchar           *preedit_string;
    IBusAttrList    *preedit_attrs;
    gint            preedit_cursor;
    gboolean        preedit_visible;
    gboolean        preedit_started;
    gint            onspot_preedit_length;
};

typedef struct _X11IC    X11IC;

static void _xim_set_cursor_location (X11IC *x11ic);

static GHashTable     *_x11_ic_table = NULL;
static GHashTable     *_ibus_ic_table = NULL;
static GHashTable     *_connections = NULL;
static XIMS _xims = NULL;
static gchar _server_name[128] = "ibus";
static gchar _locale[1024] =
    "aa,af,am,an,ar,as,az,be,bg,bn,br,bs,"
    "ca,cs,cy,da,de,dz,el,en,es,et,eu,"
    "fa,fi,fo,fr,fy,ga,gd,gl,gu,gv,"
    "he,hi,hr,hu,hy,id,is,it,iw,ja,"
    "ka,kk,kl,km,kn,ko,ku,kw,ky,lg,lo,lt,lv,"
    "mg,mi,mk,ml,mn,mr,ms,mt,nb,ne,nl,nn,no,nr,"
    "oc,om,or,pa,pl,pt,ro,ru,rw,"
    "se,si,sk,sl,so,sq,sr,ss,st,sv,"
    "ta,te,tg,th,ti,tl,tn,tr,ts,tt,"
    "uk,ur,uz,ve,vi,wa,xh,yi,zh,zu";

static gboolean _kill_daemon = FALSE;
static gint        g_debug_level = 0;

static IBusIMClient *_client = NULL;

static void
_xim_preedit_start (XIMS xims, const X11IC *x11ic)
{
    IMPreeditStateStruct ips;
    ips.major_code = 0;
    ips.minor_code = 0;
    ips.icid = x11ic->icid;
    ips.connect_id = x11ic->connect_id;
    IMPreeditStart (xims, (XPointer)&ips);
}

static void
_xim_preedit_end (XIMS xims, const X11IC *x11ic)
{
    IMPreeditStateStruct ips;
    ips.major_code = 0;
    ips.minor_code = 0;
    ips.icid = x11ic->icid;
    ips.connect_id = x11ic->connect_id;
    IMPreeditEnd (xims, (XPointer)&ips);
}


static void
_xim_preedit_callback_start (XIMS xims, const X11IC *x11ic)
{
    IMPreeditCBStruct pcb;

    pcb.major_code        = XIM_PREEDIT_START;
    pcb.minor_code        = 0;
    pcb.connect_id        = x11ic->connect_id;
    pcb.icid              = x11ic->icid;
    pcb.todo.return_value = 0;
    IMCallCallback (xims, (XPointer) & pcb);
}


static void
_xim_preedit_callback_done (XIMS xims, const X11IC *x11ic)
{
    IMPreeditCBStruct pcb;

    pcb.major_code        = XIM_PREEDIT_DONE;
    pcb.minor_code        = 0;
    pcb.connect_id        = x11ic->connect_id;
    pcb.icid              = x11ic->icid;
    pcb.todo.return_value = 0;
    IMCallCallback (xims, (XPointer) & pcb);
}


static void
_xim_preedit_callback_draw (XIMS xims, X11IC *x11ic, const gchar *preedit_string, IBusAttrList *attr_list)
{
    IMPreeditCBStruct pcb;
    XIMText text;
    XTextProperty tp;

    static XIMFeedback *feedback;
    static gint feedback_len = 0;
    guint j, i, len;

    if (preedit_string == NULL)
        return;

    len = g_utf8_strlen (preedit_string, -1);

    if (len + 1 > feedback_len) {
        feedback_len = (len + 1 + 63) & ~63;
        if (feedback) {
            feedback = g_renew (XIMFeedback, feedback, feedback_len);
        }
        else {
            feedback = g_new (XIMFeedback, feedback_len);
        }
    }

    for (i = 0; i < len; i++) {
        feedback[i] = 0;
    }

    if (attr_list != NULL) {
        for (i = 0;; i++) {
            XIMFeedback attr = 0;
            IBusAttribute *ibus_attr = ibus_attr_list_get (attr_list, i);
            if (ibus_attr == NULL) {
                break;
            }
            switch (ibus_attr->type) {
            case IBUS_ATTR_TYPE_UNDERLINE:
                if (ibus_attr->value == IBUS_ATTR_UNDERLINE_SINGLE) {
                    attr = XIMUnderline;
                }
                break;
            case IBUS_ATTR_TYPE_BACKGROUND:
                {
                    if (ibus_attr->value != 0xffffff) {
                        attr = XIMReverse;
                    }
                    break;
                }
            default:
                continue;
            }
            for (j = ibus_attr->start_index; j < ibus_attr->end_index; j++) {
                feedback[j] |= attr;
            }
        }
    }

    for (i = 0; i < len; i++) {
        if (feedback[i] == 0) {
            feedback[i] = XIMUnderline;
        }
    }
    feedback[len] = 0;

    pcb.major_code = XIM_PREEDIT_DRAW;
    pcb.connect_id = x11ic->connect_id;
    pcb.icid = x11ic->icid;

    pcb.todo.draw.caret = len;
    pcb.todo.draw.chg_first = 0;
    pcb.todo.draw.chg_length = x11ic->onspot_preedit_length;
    pcb.todo.draw.text = &text;

    text.feedback = feedback;

    if (len > 0) {
        Xutf8TextListToTextProperty (GDK_DISPLAY (), (char **)&preedit_string, 1, XCompoundTextStyle, &tp);
        text.encoding_is_wchar = 0;
        text.length = strlen ((char*)tp.value);
        text.string.multi_byte = (char*)tp.value;
        IMCallCallback (xims, (XPointer) & pcb);
        XFree (tp.value);
    } else {
        text.encoding_is_wchar = 0;
        text.length = 0;
        text.string.multi_byte = "";
        IMCallCallback (xims, (XPointer) & pcb);
        len = 0;
    }
    x11ic->onspot_preedit_length = len;
}

int
_xim_store_ic_values (X11IC *x11ic, IMChangeICStruct *call_data)
{
    XICAttribute *ic_attr = call_data->ic_attr;
    XICAttribute *pre_attr = call_data->preedit_attr;
    XICAttribute *sts_attr = call_data->status_attr;

    gint i;
    guint32 attrs = 1;

    g_return_val_if_fail (x11ic != NULL, 0);
#define _is_attr(a, b)    (g_strcmp0(a, b->name) == 0)
    for (i=0; i< (int) call_data->ic_attr_num; ++i, ++ic_attr) {
        if (_is_attr (XNInputStyle, ic_attr)) {
            x11ic->input_style = *(gint32 *) ic_attr->value;
        }
        else if (_is_attr (XNClientWindow, ic_attr)) {
            x11ic->client_window =  *(Window *) call_data->ic_attr[i].value;
        }
        else if (_is_attr (XNFocusWindow, ic_attr)) {
            x11ic->focus_window =  *(Window *) call_data->ic_attr[i].value;
        }
        else {
            // g_debug ("Unknown ic attribute: %s", ic_attr->name);
        }
    }

    for (i=0; i< (int) call_data->preedit_attr_num; ++i, ++pre_attr) {
        if (_is_attr (XNSpotLocation, pre_attr)) {
            x11ic->has_preedit_area = TRUE;
            x11ic->preedit_area.x = ((XPoint *)pre_attr->value)->x;
            x11ic->preedit_area.y = ((XPoint *)pre_attr->value)->y;
        }
        else {
            // g_debug ("Unknown preedit attribute: %s", pre_attr->name);
        }
    }

    for (i=0; i< (int) call_data->status_attr_num; ++i, ++sts_attr) {
        // g_debug ("Unkown status attribute: %s", sts_attr->name);
    }

#undef _is_attr

    return attrs;

}


int
xim_create_ic (XIMS xims, IMChangeICStruct *call_data)
{
    static int base_icid = 1;
    X11IC *x11ic;
    int i;

    LOG (1, "XIM_CREATE_IC ic=%d, connect_id=%d", call_data->icid, call_data->connect_id);

    call_data->icid = base_icid ++;

    x11ic = g_slice_new0 (X11IC);
    g_return_val_if_fail (x11ic != NULL, 0);
    x11ic->icid = call_data->icid;
    x11ic->connect_id = call_data->connect_id;
    x11ic->conn = (X11ICONN *)g_hash_table_lookup (_connections,
                        (gconstpointer)(unsigned long)call_data->connect_id);
    if (x11ic->conn == NULL) {
        g_slice_free (X11IC, x11ic);
        g_return_val_if_reached (0);
    }

    i = _xim_store_ic_values (x11ic, call_data);

    x11ic->ibus_ic = g_strdup (ibus_im_client_create_input_context (_client));
    if (x11ic->ibus_ic == NULL) {
        g_slice_free (X11IC, x11ic);
        g_return_val_if_reached (0);
    }

    g_hash_table_insert (_ibus_ic_table, x11ic->ibus_ic, (gpointer)x11ic);

    if (x11ic->input_style & XIMPreeditCallbacks) {
        ibus_im_client_set_capabilities (_client, x11ic->ibus_ic, IBUS_CAP_FOCUS | IBUS_CAP_PREEDIT);
    }
    else {
        ibus_im_client_set_capabilities (_client, x11ic->ibus_ic, IBUS_CAP_FOCUS);
    }

    g_hash_table_insert (_x11_ic_table, (gpointer)x11ic->icid, (gpointer)x11ic);
    x11ic->conn->clients = g_list_append (x11ic->conn->clients, (gpointer)x11ic);

    return 1;
}


int
xim_destroy_ic (XIMS xims, IMChangeICStruct *call_data)
{
    X11IC *x11ic;

    LOG (1, "XIM_DESTROY_IC ic=%d, connect_id=%d", call_data->icid, call_data->connect_id);

    x11ic = (X11IC *)g_hash_table_lookup (_x11_ic_table,
                (gconstpointer)(unsigned long)call_data->icid);
    g_return_val_if_fail (x11ic != NULL, 0);

    ibus_im_client_release_input_context (_client, x11ic->ibus_ic);

    g_hash_table_remove (_ibus_ic_table, x11ic->ibus_ic);
    g_hash_table_remove (_x11_ic_table,
                (gconstpointer)(unsigned long)call_data->icid);
    x11ic->conn->clients = g_list_remove (x11ic->conn->clients, (gconstpointer)x11ic);

    g_free (x11ic->preedit_string);
    ibus_attr_list_unref (x11ic->preedit_attrs);
    g_free (x11ic->ibus_ic);

    g_slice_free (X11IC, x11ic);

    return 1;
}

int
xim_set_ic_focus (XIMS xims, IMChangeFocusStruct *call_data)
{
    X11IC *x11ic;

    LOG (1, "XIM_SET_IC_FOCUS ic=%d, connect_id=%d", call_data->icid, call_data->connect_id);

    x11ic = (X11IC *)g_hash_table_lookup (_x11_ic_table,
                (gconstpointer)(unsigned long)call_data->icid);
    g_return_val_if_fail (x11ic != NULL, 0);

    ibus_im_client_focus_in (_client, x11ic->ibus_ic);
    _xim_set_cursor_location (x11ic);

    return 1;
}

int
xim_unset_ic_focus (XIMS xims, IMChangeFocusStruct *call_data)
{
    X11IC *x11ic;

    LOG (1, "XIM_UNSET_IC_FOCUS ic=%d, connect_id=%d", call_data->icid, call_data->connect_id);

    x11ic = (X11IC *)g_hash_table_lookup (_x11_ic_table,
            (gconstpointer)(unsigned long)call_data->icid);
    g_return_val_if_fail (x11ic != NULL, 0);

    ibus_im_client_focus_out (_client, x11ic->ibus_ic);

    return 1;

}

int
xim_forward_event (XIMS xims, IMForwardEventStruct *call_data)
{
    X11IC *x11ic;
    XKeyEvent *xevent;
    GdkEventKey event;

    LOG (1, "XIM_FORWARD_EVENT ic=%d, connect_id=%d", call_data->icid, call_data->connect_id);

    x11ic = (X11IC *)g_hash_table_lookup (_x11_ic_table,
                (gconstpointer)(unsigned long)call_data->icid);
    g_return_val_if_fail (x11ic != NULL, 0);

    xevent = (XKeyEvent*) &(call_data->event);

    translate_key_event (gdk_display_get_default (),
        (GdkEvent *)&event, (XEvent *)xevent);

    event.send_event = xevent->send_event;
    event.window = NULL;

    if (ibus_im_client_filter_keypress (_client, x11ic->ibus_ic, &event, False)) {
        if (! x11ic->has_preedit_area)
            _xim_set_cursor_location (x11ic);
        return 1;
    }

    IMForwardEventStruct fe;
    memset (&fe, 0, sizeof (fe));

    fe.major_code = XIM_FORWARD_EVENT;
    fe.icid = x11ic->icid;
    fe.connect_id = x11ic->connect_id;
    fe.sync_bit = 0;
    fe.serial_number = 0L;
    fe.event = call_data->event;

    IMForwardEvent (_xims, (XPointer) & fe);

    return 1;
}


int
xim_open (XIMS xims, IMOpenStruct *call_data)
{
    X11ICONN *conn;

    LOG (1, "XIM_OPEN connect_id=%d", call_data->connect_id);

    conn = (X11ICONN *)g_hash_table_lookup (_connections,
                (gconstpointer)(unsigned long)call_data->connect_id);
    g_return_val_if_fail (conn == NULL, 0);

    conn = g_slice_new0 (X11ICONN);

    g_hash_table_insert (_connections,
        (gpointer)(unsigned long)call_data->connect_id,
        (gpointer) conn);

    return 1;
}

static void
_free_ic (gpointer data, gpointer user_data)
{
    X11IC *x11ic = (X11IC *) data;

    g_return_if_fail (x11ic != NULL);

    g_free (x11ic->preedit_string);
    ibus_attr_list_unref (x11ic->preedit_attrs);
    g_free (x11ic->ibus_ic);

    /* Remove the IC from g_client dictionary */
    g_hash_table_remove (_ibus_ic_table,
                (gconstpointer)(unsigned long)x11ic->ibus_ic);
    g_hash_table_remove (_x11_ic_table,
                (gconstpointer)(unsigned long)x11ic->icid);

    g_slice_free (X11IC, x11ic);
}

int
xim_close (XIMS ims, IMCloseStruct *call_data)
{
    X11ICONN *conn;

    LOG (1, "XIM_CLOSE connect_id=%d", call_data->connect_id);

    conn = (X11ICONN *)g_hash_table_lookup (_connections,
                (gconstpointer)(unsigned long)call_data->connect_id);
    g_return_val_if_fail (conn != NULL, 0);

    g_list_foreach (conn->clients, _free_ic, NULL);

    g_list_free (conn->clients);

    // g_object_unref (conn->context);

    g_hash_table_remove (_connections, (gconstpointer)(unsigned long)call_data->connect_id);

    g_slice_free (X11ICONN, conn);

    return 1;
}


static void
_xim_set_cursor_location (X11IC *x11ic)
{
    g_return_if_fail (x11ic != NULL);

    GdkRectangle preedit_area = x11ic->preedit_area;

    Window w = x11ic->focus_window ?
        x11ic->focus_window :x11ic->client_window;

    if (w) {
        XWindowAttributes xwa;
        Window child;

        XGetWindowAttributes (GDK_DISPLAY(), w, &xwa);
        if (preedit_area.x <= 0 && preedit_area.y <= 0) {
             XTranslateCoordinates (GDK_DISPLAY(), w,
                xwa.root,
                0,
                xwa.height,
                &preedit_area.x,
                &preedit_area.y,
                &child);
        }
        else {
            XTranslateCoordinates (GDK_DISPLAY(), w,
                xwa.root,
                preedit_area.x,
                preedit_area.y,
                &preedit_area.x,
                &preedit_area.y,
                &child);
        }
    }

    ibus_im_client_set_cursor_location (_client,
            x11ic->ibus_ic, &preedit_area);
}


int
xim_set_ic_values (XIMS xims, IMChangeICStruct *call_data)
{
    X11IC *x11ic;
    gint i;

    LOG (1, "XIM_SET_IC_VALUES ic=%d connect_id=%d", call_data->icid, call_data->connect_id);

    x11ic = (X11IC *)g_hash_table_lookup (_x11_ic_table,
                (gconstpointer)(unsigned long)call_data->icid);
    g_return_val_if_fail (x11ic != NULL, 0);

    i = _xim_store_ic_values (x11ic, call_data);

    if (i) {
        _xim_set_cursor_location (x11ic);
    }

    return i;
}


int
xim_reset_ic (XIMS xims, IMResetICStruct *call_data)
{
    X11IC *x11ic;

    LOG (1, "XIM_RESET_IC ic=%d connect_id=%d", call_data->icid, call_data->connect_id);

    x11ic = (X11IC *)g_hash_table_lookup (_x11_ic_table,
                (gconstpointer)(unsigned long)call_data->icid);
    g_return_val_if_fail (x11ic != NULL, 0);

    ibus_im_client_reset (_client, x11ic->ibus_ic);

    return 1;
}

int
ims_protocol_handler (XIMS xims, IMProtocol *call_data)
{
    g_return_val_if_fail (xims != NULL, 1);
    g_return_val_if_fail (call_data != NULL, 1);

    switch (call_data->major_code) {
    case XIM_OPEN:
        return xim_open (xims, (IMOpenStruct *)call_data);
    case XIM_CLOSE:
        return xim_close (xims, (IMCloseStruct *)call_data);
    case XIM_CREATE_IC:
        return xim_create_ic (xims, (IMChangeICStruct *)call_data);
    case XIM_DESTROY_IC:
        return xim_destroy_ic (xims, (IMChangeICStruct *)call_data);
    case XIM_SET_IC_VALUES:
        return xim_set_ic_values (xims, (IMChangeICStruct *)call_data);
    case XIM_GET_IC_VALUES:
        return 1;
    case XIM_FORWARD_EVENT:
        return xim_forward_event (xims, (IMForwardEventStruct *)call_data);
    case XIM_SET_IC_FOCUS:
        return xim_set_ic_focus (xims, (IMChangeFocusStruct *)call_data);
    case XIM_UNSET_IC_FOCUS:
        return xim_unset_ic_focus (xims, (IMChangeFocusStruct *)call_data);
    case XIM_RESET_IC:
        return xim_reset_ic (xims, (IMResetICStruct *)call_data);
    case XIM_TRIGGER_NOTIFY:
    case XIM_PREEDIT_START_REPLY:
    case XIM_PREEDIT_CARET_REPLY:
    case XIM_SYNC_REPLY:
        return 1;
    default:
        break;
    }
    return 1;
}


static void
_xim_forward_gdk_event (GdkEventKey *event, X11IC *x11ic)
{
    g_return_if_fail (x11ic != NULL);

    IMForwardEventStruct fe = {0};
    XEvent xkp = {0};

    xkp.xkey.type = (event->type == GDK_KEY_PRESS) ? KeyPress : KeyRelease;
    xkp.xkey.serial = 0L;
    xkp.xkey.send_event = False;
    xkp.xkey.same_screen = True;
    xkp.xkey.display = GDK_DISPLAY();
    xkp.xkey.window = x11ic->focus_window ? x11ic->focus_window : x11ic->client_window;
    xkp.xkey.subwindow = None;
    xkp.xkey.root = DefaultRootWindow (GDK_DISPLAY());

    /*
    GTimeVal time;
    g_get_current_time (&time);
    xkp.xkey.time = time.tv_sec * 1000 + time.tv_usec / 1000;
    */
    xkp.xkey.time = 0;
    xkp.xkey.state = event->state;
    xkp.xkey.keycode = event->hardware_keycode;

    fe.major_code = XIM_FORWARD_EVENT;
    fe.icid = x11ic->icid;
    fe.connect_id = x11ic->connect_id;
    fe.sync_bit = 0;
    fe.serial_number = 0L;
    fe.event = xkp;

    IMForwardEvent (_xims, (XPointer) & fe);

}

#if 0
static void
_client_connected_cb (IBusIMClient *client, gpointer user_data)
{
}
#endif

static void
_client_disconnected_cb (IBusIMClient *client, gpointer user_data)
{
    g_warning ("Connection closed by ibus-daemon");
    exit(EXIT_SUCCESS);
}

static void
_client_commit_string_cb (IBusIMClient *client, const gchar *ic, const gchar *string, gpointer user_data)
{
    X11IC *x11ic = g_hash_table_lookup (_ibus_ic_table, ic);
    g_return_if_fail (x11ic != NULL);

    XTextProperty tp;
    IMCommitStruct cms = {0};

    Xutf8TextListToTextProperty (GDK_DISPLAY (), (char **)&string, 1, XCompoundTextStyle, &tp);

    cms.major_code = XIM_COMMIT;
    cms.icid = x11ic->icid;
    cms.connect_id = x11ic->connect_id;
    cms.flag = XimLookupChars;
    cms.commit_string = (char *)tp.value;
    IMCommitString (_xims, (XPointer) & cms);

    XFree (tp.value);

}

static void
_client_forward_event_cb (IBusIMClient *client, const gchar *ic, GdkEvent *event, gpointer user_data)
{
    X11IC *x11ic = g_hash_table_lookup (_ibus_ic_table, ic);
    g_return_if_fail (x11ic != NULL);

    _xim_forward_gdk_event (&(event->key), x11ic);
}

static void
_update_preedit (X11IC *x11ic)
{
    if (x11ic->preedit_visible == FALSE && x11ic->preedit_started == TRUE) {
        _xim_preedit_callback_draw (_xims, x11ic, "", NULL);
        _xim_preedit_callback_done (_xims, x11ic);
        x11ic->preedit_started = FALSE;
    }

    if (x11ic->preedit_visible == TRUE && x11ic->preedit_started == FALSE) {
        _xim_preedit_callback_start (_xims, x11ic);
        x11ic->preedit_started = TRUE;
    }
    if (x11ic->preedit_visible == TRUE) {
        _xim_preedit_callback_draw (_xims, x11ic, x11ic->preedit_string, x11ic->preedit_attrs);
    }
}

static void
_client_update_preedit_cb (IBusIMClient *client, const gchar *ic, const gchar *string,
    IBusAttrList *attrs, gint cursor_pos, gboolean visible, gpointer user_data)
{
    X11IC *x11ic = g_hash_table_lookup (_ibus_ic_table, ic);
    g_return_if_fail (x11ic != NULL);

    if (x11ic->preedit_string) {
        g_free(x11ic->preedit_string);
    }
    x11ic->preedit_string = g_strdup(string);

    if (x11ic->preedit_attrs) {
        ibus_attr_list_unref (x11ic->preedit_attrs);
    }
    x11ic->preedit_attrs = ibus_attr_list_ref(attrs);

    x11ic->preedit_cursor = cursor_pos;
    x11ic->preedit_visible = visible;

    _update_preedit (x11ic);
}

static void
_client_show_preedit_cb (IBusIMClient *client, const gchar *ic, gpointer user_data)
{
    X11IC *x11ic = g_hash_table_lookup (_ibus_ic_table, ic);
    g_return_if_fail (x11ic != NULL);

    x11ic->preedit_visible = TRUE;
    _update_preedit (x11ic);
}

static void
_client_hide_preedit_cb (IBusIMClient *client, const gchar *ic, gpointer user_data)
{
    X11IC *x11ic = g_hash_table_lookup (_ibus_ic_table, ic);
    g_return_if_fail (x11ic != NULL);

    x11ic->preedit_visible = FALSE;
    _update_preedit (x11ic);
}

static void
_client_enabled_cb (IBusIMClient *client, const gchar *ic, gpointer user_data)
{
    X11IC *x11ic = g_hash_table_lookup (_ibus_ic_table, ic);
    g_return_if_fail (x11ic != NULL);

    _xim_preedit_start (_xims, x11ic);
}

static void
_client_disabled_cb (IBusIMClient *client, const gchar *ic, gpointer user_data)
{
    X11IC *x11ic = g_hash_table_lookup (_ibus_ic_table, ic);
    g_return_if_fail (x11ic != NULL);

    _xim_preedit_end (_xims, x11ic);
}

static void
_init_ibus_client (void)
{
    if (_client != NULL)
        return;

    ibus_im_client_register_type (NULL);

    _client = ibus_im_client_new ();

    if (!ibus_im_client_get_connected (_client)) {
        g_error ("Can not connect to ibus-daemon!");
    }

#if 0
    g_signal_connect (_client, "connected",
                        G_CALLBACK (_client_connected_cb), NULL);
#endif

    g_signal_connect (_client, "disconnected",
                        G_CALLBACK (_client_disconnected_cb), NULL);
    g_signal_connect (_client, "commit-string",
                        G_CALLBACK (_client_commit_string_cb), NULL);
    g_signal_connect (_client, "forward-event",
                        G_CALLBACK (_client_forward_event_cb), NULL);

    g_signal_connect (_client, "update-preedit",
                        G_CALLBACK (_client_update_preedit_cb), NULL);
    g_signal_connect (_client, "show-preedit",
                        G_CALLBACK (_client_show_preedit_cb), NULL);
    g_signal_connect (_client, "hide-preedit",
                        G_CALLBACK (_client_hide_preedit_cb), NULL);
    g_signal_connect (_client, "enabled",
                        G_CALLBACK (_client_enabled_cb), NULL);
    g_signal_connect (_client, "disabled",
                        G_CALLBACK (_client_disabled_cb), NULL);
}

static void
_xim_init_IMdkit ()
{
#if 0
    XIMStyle ims_styles_overspot [] = {
        XIMPreeditPosition  | XIMStatusNothing,
        XIMPreeditNothing   | XIMStatusNothing,
        XIMPreeditPosition  | XIMStatusCallbacks,
        XIMPreeditNothing   | XIMStatusCallbacks,
        0
    };
#endif

    XIMStyle ims_styles_onspot [] = {
        XIMPreeditPosition  | XIMStatusNothing,
        XIMPreeditCallbacks | XIMStatusNothing,
        XIMPreeditNothing   | XIMStatusNothing,
        XIMPreeditPosition  | XIMStatusCallbacks,
        XIMPreeditCallbacks | XIMStatusCallbacks,
        XIMPreeditNothing   | XIMStatusCallbacks,
        0
    };

    XIMEncoding ims_encodings[] = {
        "COMPOUND_TEXT",
        0
    };

    GdkWindowAttr window_attr = {
        title : "ibus-xim",
        event_mask : GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK,
        wclass : GDK_INPUT_OUTPUT,
        window_type : GDK_WINDOW_TOPLEVEL,
        override_redirect : 1,
    };

    XIMStyles styles;
    XIMEncodings encodings;

    GdkWindow *win;

    win = gdk_window_new (NULL, &window_attr, GDK_WA_TITLE);

    styles.count_styles =
        sizeof (ims_styles_onspot)/sizeof (XIMStyle) - 1;
    styles.supported_styles = ims_styles_onspot;

    encodings.count_encodings =
        sizeof (ims_encodings)/sizeof (XIMEncoding) - 1;
    encodings.supported_encodings = ims_encodings;

    _xims = IMOpenIM(GDK_DISPLAY(),
        IMModifiers, "Xi18n",
        IMServerWindow, GDK_WINDOW_XWINDOW(win),
        IMServerName, _server_name,
        IMLocale, _locale,
        IMServerTransport, "X/",
        IMInputStyles, &styles,
        IMEncodingList, &encodings,
        IMProtocolHandler, ims_protocol_handler,
        IMFilterEventMask, KeyPressMask | KeyReleaseMask,
        NULL);

    _init_ibus_client ();

    if (!ibus_im_client_get_connected (_client)) {
        g_warning ("Can not connect to ibus daemon");
        exit (1);
    }
}

static void
_atexit_cb ()
{
    ibus_im_client_kill_daemon(_client);
}

static void
_sighandler (int sig)
{
    exit(EXIT_FAILURE);
}

static void
_print_usage (FILE *fp, gchar *name)
{
    fprintf (fp,
        "Usage:\n"
        " %s --help               Show this message\n"
        "    --server-name= -n    Setup xim sevrer name\n"
        "    --locale= -l         Setup support locale\n"
        "    --kill-daemon -k     Kill ibus daemon when exit\n"
        "    --debug= -v          Setup debug level\n",
        name);
}

static int
_xerror_handler (Display *dpy, XErrorEvent *e)
{
    g_debug (
        "XError: "
        "serial=%lu error_code=%d request_code=%d minor_code=%d resourceid=%lu",
        e->serial, e->error_code, e->request_code, e->minor_code, e->resourceid);
    return 1;
}

int
main (int argc, char **argv)
{
    gint option_index = 0;
    gint c;

    gtk_init (&argc, &argv);
    XSetErrorHandler (_xerror_handler);

    while (1) {
        static struct option long_options [] = {
            {"debug", 1, 0, 0},
            {"server-name", 1, 0, 0},
            {"locale", 1, 0, 0},
            {"help", 0, 0, 0},
            {"kill-daemon", 0, 0, 0},
            {0, 0, 0, 0},
        };

        c = getopt_long (argc, argv, "v:n:l:k",
            long_options, &option_index);

        if (c == -1) break;

        switch (c) {
        case 0:
            if (g_strcmp0 (long_options[option_index].name, "debug") == 0) {
                g_debug_level = atoi (optarg);
            }
            else if (g_strcmp0 (long_options[option_index].name, "server-name") == 0) {
                strncpy (_server_name, optarg, sizeof (_server_name));
            }
            else if (g_strcmp0 (long_options[option_index].name, "locale") == 0) {
                strncpy (_locale, optarg, sizeof (_locale));
            }
            else if (g_strcmp0 (long_options[option_index].name, "help") == 0) {
                _print_usage (stdout, argv[0]);
                exit (EXIT_SUCCESS);
            }
            else if (g_strcmp0 (long_options[option_index].name, "kill-daemon") == 0) {
                _kill_daemon = TRUE;
            }
            break;
        case 'v':
            g_debug_level = atoi (optarg);
            break;
        case 'n':
            strncpy (_server_name, optarg, sizeof (_server_name));
            break;
        case 'l':
            strncpy (_locale, optarg, sizeof (_locale));
            break;
        case 'k':
            _kill_daemon = TRUE;
            break;
        case '?':
        default:
            _print_usage (stderr, argv[0]);
            exit (EXIT_FAILURE);
        }
    }

    _x11_ic_table = g_hash_table_new (g_direct_hash, g_direct_equal);
    _ibus_ic_table = g_hash_table_new (g_str_hash, g_str_equal);
    _connections = g_hash_table_new (g_direct_hash, g_direct_equal);

    signal (SIGINT, _sighandler);
    signal (SIGTERM, _sighandler);

    if (_kill_daemon)
        g_atexit (_atexit_cb);

    _xim_init_IMdkit ();
    gtk_main();

    exit (EXIT_SUCCESS);
}



