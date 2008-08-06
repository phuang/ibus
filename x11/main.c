/* xim-on-gtkim
 * Copyright (C) 2007-2008 Huang Peng <phuang@redhat.com>
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
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <langinfo.h>
#include <locale.h>
#include <iconv.h>

#define _GNU_SOURCES
#include <getopt.h>

#define LOG(level, fmt, args...)	\
	if (g_debug_level >= (level)) {	\
	fprintf (stderr, fmt, args);	\
	}

#include <ibusimclient.h>
#include "gdk-private.h"

struct _X11ICONN {
	GList		*clients;
};
typedef struct _X11ICONN	X11ICONN;

struct _X11IC {
	GtkIMContext *context;
	GdkWindow 	*client_window;
	GdkWindow 	*focus_window;
	gint32		input_style;
	X11ICONN	*conn;
	gint		icid;
	gint		connect_id;
	gchar		*lang;
	GdkRectangle	preedit_area;
};
typedef struct _X11IC	X11IC;



static void _xim_commit_cb (GtkIMContext *context, gchar *arg, gpointer data);

static GHashTable 	*_clients = NULL;
static GHashTable 	*_connections = NULL;
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

static gint		g_debug_level = 0;

IBusIMClient *_client = NULL;


static void
_xim_preedit_start (XIMS xims, int icid, int connect_id)
{
	IMPreeditStateStruct ips;
	ips.major_code = 0;
	ips.minor_code = 0;
	ips.icid = icid;
	ips.connect_id = connect_id;
	IMPreeditStart (xims, (XPointer)&ips);
}

static void
_xim_preedit_end (XIMS xims, int icid, int connect_id)
{
	IMPreeditStateStruct ips;
	ips.major_code = 0;
	ips.minor_code = 0;
	ips.icid = icid;
	ips.connect_id = connect_id;
	IMPreeditEnd (xims, (XPointer)&ips);
}

int
_xim_store_ic_values (X11IC *ic, IMChangeICStruct *call_data)
{
	XICAttribute *ic_attr = call_data->ic_attr;
	XICAttribute *pre_attr = call_data->preedit_attr;
	XICAttribute *sts_attr = call_data->status_attr;

	gint i;
	guint32 attrs = 1;

	if (ic == NULL) {
		return 0;
	}
#define _is_attr(a, b)	(strcmp(a, b->name) == 0)
	for (i=0; i< (int) call_data->ic_attr_num; ++i, ++ic_attr) {
		if (_is_attr (XNInputStyle, ic_attr)) {
			ic->input_style = *(gint32 *) ic_attr->value;
		}
		else if (_is_attr (XNClientWindow, ic_attr)) {
			Window w;

			if (ic->client_window != NULL) {
				g_object_unref (ic->client_window);
			}

			w =  *(Window *) call_data->ic_attr[i].value;
			ic->client_window = gdk_window_foreign_new (w);
			g_object_set_data (G_OBJECT (ic->client_window), "IC", ic);
		}
		else if (_is_attr (XNFocusWindow, ic_attr)) {
			Window w;

			if (ic->focus_window != NULL) {
				g_object_unref (ic->focus_window);
			}

			w =  *(Window *) call_data->ic_attr[i].value;
			ic->focus_window = gdk_window_foreign_new (w);
		}
		else {
			// fprintf (stderr, "Unknown attr: %s\n", ic_attr->name);
		}
	}

	for (i=0; i< (int) call_data->preedit_attr_num; ++i, ++pre_attr) {
		if (_is_attr (XNSpotLocation, pre_attr)) {
			ic->preedit_area.x = ((XPoint *)pre_attr->value)->x;
			ic->preedit_area.y = ((XPoint *)pre_attr->value)->y;
		}
		else {
			// fprintf (stderr, "Unknown attr: %s\n", pre_attr->name);
		}
	}

	for (i=0; i< (int) call_data->status_attr_num; ++i, ++sts_attr) {
		// printf ("set status: %s\n", sts_attr->name);
	}

#undef _is_attr

	return attrs;

}


static void
_xim_commit_cb (GtkIMContext *context, gchar *arg, gpointer data)
{
	char *clist[1];
	XTextProperty tp;
	IMCommitStruct cms;

	X11IC *ic = (X11IC *)data;

	clist[0] = arg;
	Xutf8TextListToTextProperty (GDK_DISPLAY (), clist, 1, XCompoundTextStyle, &tp);

	memset (&cms, 0, sizeof (cms));
	cms.major_code = XIM_COMMIT;
	cms.icid = ic->icid;
	cms.connect_id = ic->connect_id;
	cms.flag = XimLookupChars;
	cms.commit_string = (char *)tp.value;
	IMCommitString (_xims, (XPointer) & cms);

	XFree (tp.value);

}

int
xim_create_ic (XIMS xims, IMChangeICStruct *call_data)
{
	static int base_icid = 1;
	X11IC *ic;
	int i;

	LOG (1, "XIM_CREATE_IC ic=%d, connect_id=%d\n", call_data->icid, call_data->connect_id);

	call_data->icid = base_icid ++;

	ic = g_malloc0 (sizeof (X11IC));
	ic->icid = call_data->icid;
	ic->connect_id = call_data->connect_id;
	ic->conn = (X11ICONN *)g_hash_table_lookup (_connections,
						(gconstpointer)(unsigned long)call_data->connect_id);


	i = _xim_store_ic_values (ic, call_data);

	ic->context = (GtkIMContext *)ibus_im_client_create_im_context (_client);
	gtk_im_context_set_client_window (ic->context, ic->client_window);
	gtk_im_context_set_use_preedit (ic->context, FALSE);
	g_signal_connect (ic->context,
			"commit",
			G_CALLBACK (_xim_commit_cb),
			(gpointer)ic);
#if 0
	g_signal_connect (ic->context,
			"preedit-changed",
			G_CALLBACK (_xim_preedit_changed_cb),
			(gpointer)ic);
#endif

	g_hash_table_insert (_clients, (gpointer)ic->icid, (gpointer) ic);
	ic->conn->clients = g_list_append (ic->conn->clients, (gpointer) ic);

	return 1;
}


int
xim_destroy_ic (XIMS xims, IMChangeICStruct *call_data)
{
	X11IC *ic;

	LOG (1, "XIM_DESTROY_IC ic=%d, connect_id=%d\n", call_data->icid, call_data->connect_id);

	ic = (X11IC *)g_hash_table_lookup (_clients,
				(gconstpointer)(unsigned long)call_data->icid);
	g_object_unref (ic->context);
	ic->conn->clients = g_list_remove (ic->conn->clients, (gconstpointer)ic);
	g_hash_table_remove (_clients,
				(gconstpointer)(unsigned long)call_data->icid);

	g_free (ic);

	return 1;
}

int
xim_set_ic_focus (XIMS xims, IMChangeFocusStruct *call_data)
{
	X11IC *ic;

	LOG (1, "XIM_SET_IC_FOCUS ic=%d, connect_id=%d\n", call_data->icid, call_data->connect_id);

	ic = (X11IC *)g_hash_table_lookup (_clients,
				(gconstpointer)(unsigned long)call_data->icid);

	gtk_im_context_focus_in (ic->context);

	return 1;

}

int
xim_unset_ic_focus (XIMS xims, IMChangeFocusStruct *call_data)
{
	X11IC *ic;

	LOG (1, "XIM_UNSET_IC_FOCUS ic=%d, connect_id=%d\n", call_data->icid, call_data->connect_id);

	ic = (X11IC *)g_hash_table_lookup (_clients,
			(gconstpointer)(unsigned long)call_data->icid);

	gtk_im_context_focus_out (ic->context);

	return 1;

}

int
xim_forward_event (XIMS xims, IMForwardEventStruct *call_data)
{

	X11IC *ic;
	XKeyEvent *xevent;
	GdkEventKey event;
	GdkWindow *window;

	ic = (X11IC *)g_hash_table_lookup (_clients,
				(gconstpointer)(unsigned long)call_data->icid);

	g_return_val_if_fail (ic != NULL, 1);

	window = ic->focus_window != NULL ? ic->focus_window : ic->client_window;

	xevent = (XKeyEvent*) &(call_data->event);

	translate_key_event (gdk_drawable_get_display (window),
		(GdkEvent *)&event, (XEvent *)xevent);

	event.send_event = xevent->send_event;
	event.window = window;

	if (gtk_im_context_filter_keypress (ic->context, &event)) {
		return 1;
	}
	else {
		IMForwardEventStruct fe;
		XEvent xkp;
		XKeyEvent *event = (XKeyEvent*) (&xkp);

		memset (&fe, 0, sizeof (fe));
		fe.major_code = XIM_FORWARD_EVENT;
		fe.icid = ic->icid;
		fe.connect_id = ic->connect_id;
		fe.sync_bit = 0;
		fe.serial_number = 0L;
		fe.event = call_data->event;
		IMForwardEvent (_xims, (XPointer) & fe);
		return 1;
	}
}


int
xim_open (XIMS xims, IMOpenStruct *call_data)
{
	X11ICONN *conn;
	gchar *last;

	LOG (1, "XIM_OPEN connect_id=%d\n", call_data->connect_id);

	conn = (X11ICONN *)g_hash_table_lookup (_connections,
				(gconstpointer)(unsigned long)call_data->connect_id);

	g_return_val_if_fail (conn == NULL, 1);

	conn = (X11ICONN *) g_malloc0(sizeof (X11ICONN));
	// conn->context = GTK_IM_CONTEXT (gtk_im_multicontext_new ());

	g_hash_table_insert (_connections,
		(gpointer)(unsigned long)call_data->connect_id,
		(gpointer) conn);

	// g_signal_connect_after (conn->context,
	// 		"commit",
	// 		G_CALLBACK (_xim_commit_cb),
	// 		(gpointer)(unsigned long)call_data->connect_id);

	return 1;
}

static void
_free_ic (gpointer data, gpointer user_data)
{
	X11IC *ic = (X11IC *) data;

	g_return_if_fail (ic != NULL);

	g_object_unref (ic->context);

	/* Remove the IC from g_client dictionary */
	g_hash_table_remove (_clients,
				(gconstpointer)(unsigned long)ic->icid);

	g_free (ic);
}

int
xim_close (XIMS ims, IMCloseStruct *call_data)
{
	X11ICONN *conn;

	LOG (1, "XIM_CLOSE connect_id=%d\n", call_data->connect_id);

	conn = (X11ICONN *)g_hash_table_lookup (_connections,
				(gconstpointer)(unsigned long)call_data->connect_id);

	g_return_val_if_fail (conn != NULL, 1);

	g_list_foreach (conn->clients, _free_ic, NULL);

	g_list_free (conn->clients);

	// g_object_unref (conn->context);

	g_hash_table_remove (_connections, (gconstpointer)(unsigned long)call_data->connect_id);

	g_free (conn);

	return 1;
}



int
xim_set_ic_values (XIMS xims, IMChangeICStruct *call_data)
{
	X11IC *ic;
	gint i;

	LOG (1, "XIM_SET_IC_VALUES ic=%d connect_id=%d\n", call_data->icid, call_data->connect_id);

	ic = (X11IC *)g_hash_table_lookup (_clients,
				(gconstpointer)(unsigned long)call_data->icid);

	g_return_val_if_fail (ic != NULL, 1);

	i = _xim_store_ic_values (ic, call_data);

	if (i) {
		gtk_im_context_set_cursor_location (ic->context, &ic->preedit_area);
	}

	return i;
}


int
xim_reset_ic (XIMS xims, IMResetICStruct *call_data)
{
	X11IC *ic;

	LOG (1, "XIM_RESET_IC ic=%d connect_id=%d\n", call_data->icid, call_data->connect_id);

	ic = (X11IC *)g_hash_table_lookup (_clients,
				(gconstpointer)(unsigned long)call_data->icid);

	g_return_val_if_fail (ic != NULL, 1);

	gtk_im_context_reset (ic->context);

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
_xim_forward_gdk_event (GdkEventKey *event)
{
	X11IC *ic;
	ic = (X11IC *)g_object_get_data (G_OBJECT (event->window), "IC");
	IMForwardEventStruct fe;
	XEvent xkp;
	memset (&xkp, 0, sizeof (xkp));
	memset (&fe, 0, sizeof (fe));

	xkp.xkey.type = (event->type == GDK_KEY_PRESS) ? KeyPress : KeyRelease;
	xkp.xkey.serial = 0L;
	xkp.xkey.send_event = False;
	xkp.xkey.same_screen = False;
	xkp.xkey.display = GDK_WINDOW_XDISPLAY (event->window);
	xkp.xkey.window = GDK_WINDOW_XWINDOW (event->window);
	xkp.xkey.subwindow = None;
	xkp.xkey.root = DefaultRootWindow (GDK_WINDOW_XDISPLAY (event->window));
	xkp.xkey.time = event->time;
	xkp.xkey.state = event->state;
	xkp.xkey.keycode = event->hardware_keycode;

	fe.major_code = XIM_FORWARD_EVENT;
	fe.icid = ic->icid;
	fe.connect_id = ic->connect_id;
	fe.sync_bit = 0;
	fe.serial_number = 0L;
	fe.event = xkp;
	IMForwardEvent (_xims, (XPointer) & fe);

}

static void
_xim_event_cb (GdkEvent *event, gpointer data)
{
	switch (event->type) {
	case GDK_KEY_PRESS:
	case GDK_KEY_RELEASE:
		_xim_forward_gdk_event ((GdkEventKey *)event);
		break;
	default:
		gtk_main_do_event (event);
		break;
	}
}

static void
_xim_event_destroy_cb (gpointer data)
{
}

static void
_xim_init_IMdkit ()
{
	XIMStyle ims_styles_overspot [] = {
		XIMPreeditPosition  | XIMStatusNothing,
		XIMPreeditNothing   | XIMStatusNothing,
		XIMPreeditPosition  | XIMStatusCallbacks,
		XIMPreeditNothing   | XIMStatusCallbacks,
		0
	};

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
		title : 	"xim2gtkim",
		event_mask : 	GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK,
		wclass:		GDK_INPUT_OUTPUT,
		window_type:	GDK_WINDOW_TOPLEVEL,
		override_redirect: 1,
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

	gdk_event_handler_set (_xim_event_cb, NULL,
		_xim_event_destroy_cb);

	ibus_im_client_register_type (NULL);
	ibus_im_context_register_type (NULL);
	_client = ibus_im_client_new ();

}

static void
print_usage (FILE *fp, gchar *name)
{
	fprintf (fp,
		"Usage:\n"
		" %s --help               Show this message\n"
		"    --server-name= -n    Setup xim sevrer name\n"
		"    --locale= -l         Setup support locale\n"
		"    --debug= -v          Setup debug level\n",
		name);
}


int main (int argc, char **argv)
{
	gint option_index = 0;
	gint c;

	
	gtk_init (&argc, &argv);

	while (1) {
		static struct option long_options [] = {
			{"debug", 1, 0, 0},
			{"server-name", 1, 0, 0},
			{"locale", 1, 0, 0},
			{"help", 0, 0, 0},
			{0, 0, 0, 0},
		};

		c = getopt_long (argc, argv, "v:n:l:",
			long_options, &option_index);

		if (c == -1) break;

		switch (c) {
		case 0:
			if (strcmp (long_options[option_index].name, "debug") == 0) {
				g_debug_level = atoi (optarg);
			}
			else if (strcmp (long_options[option_index].name, "server-name") == 0) {
				strncpy (_server_name, optarg, sizeof (_server_name));
			}
			else if (strcmp (long_options[option_index].name, "locale") == 0) {
				strncpy (_locale, optarg, sizeof (_locale));
			}
			else if (strcmp (long_options[option_index].name, "help") == 0) {
				print_usage (stdout, argv[0]);
				exit (EXIT_SUCCESS);
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
		case '?':
		default:
			print_usage (stderr, argv[0]);
			exit (EXIT_FAILURE);
		}


	}

	_clients = g_hash_table_new (g_direct_hash, g_direct_equal);
	_connections = g_hash_table_new (g_direct_hash, g_direct_equal);

	// printf ("server-name = %s\n", _server_name);
	// printf ("locale      = %s\n", g_locale);

	_xim_init_IMdkit ();
	
	gtk_main();

	return 0;

}
