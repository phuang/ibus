/* vim:set et ts=4: */
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

%module IMdkit
%{
#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <Xi18n.h>
#include <pygobject.h>
#include <pygtk/pygtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
%}

%init %{
    pygobject_init (-1, -1, -1);
    init_pygtk ();
    /*
    gint argc = 1;
    gchar *argv [] = { NULL, NULL};
    argv[0] = g_strdup (Py_GetProgramName ());
    g_debug ("1 %s", argv[0]);
    gdk_init (&argc, (gchar ***)&argv);
    g_debug ("2");
    g_debug ("3");
    */
%}

%{
/* define XIM structure */
struct XIM {
    struct XIMS *xims;
    PyObject *window;
    PyObject *protocol_handler;
};

static struct XIM * _xim = NULL;

/* define protocol handler */
static int
_ims_protocol_handler (XIMS xims, IMProtocol *call_data)
{
    PyObject *xim = NULL;

    if (_xim == NULL || _xim->xims != xims)
        return 0;
    
    return 0;
}

%}

/* define exception */
%exception {
    $action;
    if (PyErr_Occurred ()) {
        return NULL;
    }
}

/* define type maps */
%typemap (in) PyObject * {
    $1 = $input;
}

%typemap (out) PyObject * {
    $result = $1;
}

struct XIM {
    /* define property */
    %immutable;
    PyObject *window;
    %mutable;
};

%extend XIM {
    XIM (char *name, char *locale, PyObject *protocol_handler) {
        struct XIM *self = NULL;

        if (name == NULL) {
            PyErr_Format (PyExc_TypeError,
                "Argument 1 of XIM must be a string.");
            goto failed;
        }
        
        if (locale == NULL) {
            PyErr_Format (PyExc_TypeError,
                "Argument 2 of XIM must be a string.");
            goto failed;
        }

        if (!PyCallable_Check (protocol_handler)) {
            PyErr_Format (PyExc_TypeError,
                "Argument 3 of XIM must be a callable object.");
            goto failed;
        }
        
        if (_xim != NULL) {
            PyErr_Format (PyExc_RuntimeError,
                "XIM can not be created second time.");
            goto failed;
            
        }

        self = g_new (struct XIM, 1);

        Py_INCREF (protocol_handler);
        self->protocol_handler = protocol_handler;

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
            title :     "xim2gtkim",
            event_mask :    GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK,
            wclass:     GDK_INPUT_OUTPUT,
            window_type:    GDK_WINDOW_TOPLEVEL,
            override_redirect: 1,
        };

        GdkWindow *window = gdk_window_new (NULL, &window_attr, GDK_WA_TITLE);
        self->window = pygobject_new (window);
        Py_INCREF (self->window);
        
        XIMStyles styles;
        XIMEncodings encodings;

        styles.count_styles =
            sizeof (ims_styles_onspot)/sizeof (XIMStyle) - 1;
        styles.supported_styles = ims_styles_onspot;
        
        encodings.count_encodings =
            sizeof (ims_encodings)/sizeof (XIMEncoding) - 1;
        encodings.supported_encodings = ims_encodings;
        
        self->xims = IMOpenIM (GDK_DISPLAY (),
            IMModifiers, "Xi18n",
            IMServerWindow, GDK_WINDOW_XWINDOW (window),
            IMServerName, name,
            IMLocale, locale,
            IMServerTransport, "X/",
            IMInputStyles, &styles,
            IMEncodingList, &encodings,
            IMProtocolHandler, _ims_protocol_handler,
            IMFilterEventMask, KeyPressMask | KeyReleaseMask,
            NULL
            );

        _xim = self;
        return self;

        failed:
        if (self) {
            Py_XDECREF (self->window);
            Py_XDECREF (self->protocol_handler);
            g_free (self);
        }
        return NULL;
    }

    ~XIM () {
        if (self) {
            if (self->xims) {
                IMCloseIM (self->xims);
            }
            Py_XDECREF (self->window);
            Py_XDECREF (self->protocol_handler);
            g_free (self);
        }
        _xim = NULL;
    }

}

