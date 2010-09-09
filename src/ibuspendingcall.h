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
 * SECTION: ibuspendingcall
 * @title: IBusPendingCall
 * @short_description: A DBusPendingCall in IBus.
 * @stability: Stable
 *
 * An IBusPendingCall is essentially a DBusPendingCall, which representing an expected reply.
 * A IBusPendingCall can be created when you send a message that should have a reply.
 *
 * Besides DBusPendingCall functions, An IBusPendingCall can be manipulated
 * with its own specific functions, which are defined in this section.
 */
#ifndef __IBUS_PENDING_CALL_H_
#define __IBUS_PENDING_CALL_H_

#include <glib.h>
#include "ibusdbus.h"
#include "ibusmessage.h"

G_BEGIN_DECLS

/**
 * IBusPendingCall:
 *
 * An opaque data structure that represents IBusPendingCall.
 */
typedef DBusPendingCall IBusPendingCall;

/**
 * IBusPendingCallNotifyFunction:
 * @pending: An IBusPendingCall.
 * @user_data: User data for the callback function.
 *
 * Callback prototype of pending call notify function.
 */
typedef void (* IBusPendingCallNotifyFunction)(IBusPendingCall *pending, gpointer user_data);

/**
 * ibus_pending_call_ref:
 * @pending: An IBusPendingCall.
 * @returns: A reference of IBusPendingCall.
 *
 * Increases the reference count on a pending call.
 */
IBusPendingCall*    ibus_pending_call_ref           (IBusPendingCall                *pending);

/**
 * ibus_pending_call_unref:
 * @pending: An IBusPendingCall.
 *
 * Decreases the reference count on a pending call.
 */
void                ibus_pending_call_unref         (IBusPendingCall                *pending);

/**
 * ibus_pending_call_set_notify:
 * @pending: An IBusPendingCall.
 * @function: An pending call notify callback function.
 * @user_data: User data for the callback function.
 * @free_user_data: Callback to free the user_data.
 * @returns: TRUE if succeed; FALSE if not enough memory.
 *
 * Sets a notification function to be called when the reply is received or the pending call times out.
 */
gboolean            ibus_pending_call_set_notify    (IBusPendingCall                *pending,
                                                     IBusPendingCallNotifyFunction   function,
                                                     gpointer                        user_data,
                                                     GDestroyNotify                  free_user_data);

/**
 * ibus_pending_call_cancel:
 * @pending: An IBusPendingCall.
 *
 * Cancels the pending call, such that any reply or error received will just be ignored.
 *
 * Drops the dbus library's internal reference to the DBusPendingCall so will free the call
 * if nobody else is holding a reference.
 * But usually application owns a reference from dbus_connection_send_with_reply().
 *
 * Note that canceling a pending call will not simulate a timed-out call;
 * if a call times out, then a timeout error reply is received.
 * If you cancel the call, no reply is received unless the reply was already received before you canceled.
 */
void                ibus_pending_call_cancel        (IBusPendingCall                *pending);

/**
 * ibus_pending_call_get_completed:
 * @pending: An IBusPendingCall.
 * @returns: TRUE if pending call has received a reply; FALSE otherwise.
 *
 * Whether the pending call has received a reply or not.
 */
gboolean            ibus_pending_call_get_completed (IBusPendingCall                *pending);

/**
 * ibus_pending_call_steal_reply:
 * @pending: An IBusPendingCall.
 * @returns: Replied message; NULL if none has been received yet.
 *
 * Gets the reply, or returns NULL if none has been received yet.
 *
 * Ownership of the reply message passes to the caller.
 * This function can only be called once per pending call,
 * since the reply message is transferred to the caller.
 */
IBusMessage*        ibus_pending_call_steal_reply   (IBusPendingCall                *pending);

/**
 * ibus_pending_call_block:
 * @pending: An IBusPendingCall.
 *
 * Block until the pending call is completed.
 * The blocking is as with ibus_connection_send_with_reply_and_block();
 * it does not enter the main loop or process other messages,
 * it simply waits for the reply in question.
 *
 * If the pending call is already completed, this function returns immediately.
 */
void                ibus_pending_call_block         (IBusPendingCall                *pending);

/**
 * ibus_pending_call_wait:
 * @pending: An IBusPendingCall.
 *
 * Wait until the pending call is completed.
 *
 * See also: ibus_pending_call_get_completed().
 */
void                ibus_pending_call_wait          (IBusPendingCall                *pending);

/**
 * ibus_pending_call_allocate_data_slot:
 * @slot_p: Address of a global variable storing the slot.
 * @returns: TRUE if succeed; FALSE if insufficient memory.
 *
 * Allocates an integer ID to be used for storing application-specific data on any IBusPendingCall.
 *
 * The allocated ID may then be used with ibus_pending_call_set_data() and ibus_pending_call_get_data().
 * The passed-in slot must be initialized to -1, and is filled in with the slot ID.
 * If the passed-in slot is not -1, it's assumed to be already allocated, and
 * its reference count is increased.
 *
 * The allocated slot is global, i.e. all DBusPendingCall objects
 * will have a slot with the given integer ID reserved.
 */
gboolean            ibus_pending_call_allocate_data_slot
                                                    (gint                           *slot_p);

/**
 * ibus_pending_call_free_data_slot:
 * @slot_p: Address of a global variable storing the slot.
 *
 * Deallocates a global ID for IBusPendingCall data slots.
 *
 * ibus_pending_call_get_data() and ibus_pending_call_set_data() may no longer be used with this slot.
 * Existing data stored on existing IBusPendingCall objects will be freed when
 * the IBusPendingCall is finalized, but may not be retrieved
 * (and may only be replaced if someone else reallocates the slot).
 * When the reference count on the passed-in slot reaches 0, it is set to -1.
 */
void                ibus_pending_call_free_data_slot
                                                    (gint                           *slot_p);

/**
 * ibus_pending_call_set_data:
 * @pending: An IBusPendingCall.
 * @slot: The slot number.
 * @data: The data to store
 * @free_data_func: Callback to free the data.
 * @returns: TRUE if there was enough memory to store the data; FALSE otherwise.
 *
 * Stores a pointer on a IBusPendingCall, along with an optional function
 * to be used for freeing the data when the data is set again, or when the pending call is finalized.
 *
 * The slot number must have been allocated with ibus_pending_call_allocate_data_slot().
 */
gboolean            ibus_pending_call_set_data      (IBusPendingCall                *pending,
                                                     gint                            slot,
                                                     gpointer                        data,
                                                     GDestroyNotify                  free_data_func);

/**
 * ibus_pending_call_get_data:
 * @pending: An IBusPendingCall.
 * @slot: The slot number.
 * @returns: The stored data; NULL if no such data.
 *
 * Retrieves data previously set with ibus_pending_call_set_data().
 *
 * The slot must still be allocated (must not have been freed).
 */
gpointer            ibus_pending_call_get_data      (IBusPendingCall                *pending,
                                                     gint                            slot);

G_END_DECLS
#endif
