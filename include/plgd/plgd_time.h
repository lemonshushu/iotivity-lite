/****************************************************************************
 *
 * Copyright 2023 Daniel Adam, All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"),
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/

/**
 * @file plgd_time.h
 *
 * @brief Time synchronization from an external endpoint.
 *
 * @author Daniel Adam
 */

#ifndef PLGD_TIME_H
#define PLGD_TIME_H

#include "util/oc_features.h"

#ifdef OC_HAS_FEATURE_PLGD_TIME

/**
 * \defgroup time_synchronization Time synchronization
 *
 * A facitility to synchronize time for SSL certificates validation or for the
 * whole system.
 *
 * @{
 */

#include "oc_endpoint.h"
#include "oc_export.h"
#include "oc_ri.h"
#include "port/oc_clock.h"
#include "util/oc_compiler.h"

#if defined(OC_SECURITY) && defined(OC_PKI)
#include "oc_pki.h"
#endif /* OC_SECURITY && OC_PKI */

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  /* UNINITIALIZED = 0, */
  PLGD_TIME_STATUS_SYNCING = 1,
  PLGD_TIME_STATUS_IN_SYNC,
  PLGD_TIME_STATUS_IN_SYNC_FROM_STORAGE, // special status value, used when time
                                         // is loaded from storage
} plgd_time_status_t;

/**
 * @brief Function used to set time on the whole system invoked whenever the
 * plgd time is updated.
 *
 * @param time current time
 * @param user_data custom user data passed from the caller
 */
typedef int (*plgd_set_system_time_fn_t)(oc_clock_time_t time, void *user_data);

/**
 * @brief Configure the plgd time feature.
 *
 * @param use_in_mbedtls propagate the oc_plgd_time function to mbedTLS, if
 * false then the standard time function will be used in mbedTLS (only used in
 * OC_SECURE builds)
 * @param set_system_time function used to set time on the whole system (for
 * example: a wrapper over the settimeofday function on Linux) whenever the
 * plgd time is modified (by plgd_time_set_time or a POST request)
 * @param set_system_time_data user data passed to set_system_time
 *
 * @note to report synchronization status use plgd_time_set_status
 *
 * @see plgd_time_set_time
 */
OC_API
void plgd_time_configure(bool use_in_mbedtls,
                         plgd_set_system_time_fn_t set_system_time,
                         void *set_system_time_data);

/**
 * @brief Plgd time is active (ie it is set to a valid, non-zero value).
 *
 * @return true plgd time is synchronized
 * @return false otherwise
 *
 * @see plgd_time_set_time
 */
OC_API
bool plgd_time_is_active(void);

/**
 * @brief Calculate current plgd time.
 *
 * The plgd time is calculated by adding synchronization time and elapsed
 * time since the synchronization.
 * The value should represent number of ticks since the Unix Epoch (1970-01-01
 * 00:00:00 +0000 UTC).
 *
 * @return >=0 number of system ticks since the Unix Epoch
 * @return -1 on error
 *
 * @see plgd_time_set_time
 */
OC_API
oc_clock_time_t plgd_time(void);

/** @brief Calculate the number of seconds since the Unix Epoch */
OC_API
unsigned long plgd_time_seconds(void);

/**
 * @brief Synchronize the plgd time.
 *
 * Store the synchronization time and the monotonic time of the synchronization.
 * The plgd time is then calculated as synchronization time + time since
 * elapsed since synchronization.
 *
 * @param time synchronization time
 * @return 0 on success
 * @return -1 on failure
 *
 * @note on successful call the plgd time status is set to
 * PLGD_TIME_STATUS_IN_SYNC
 */
OC_API
int plgd_time_set_time(oc_clock_time_t time);

/** @brief Get the latest synchronization time */
OC_API
oc_clock_time_t plgd_time_last_synced_time(void);

/** @brief Set plgd time status */
OC_API
void plgd_time_set_status(plgd_time_status_t status);

/** @brief Get plgd time status */
OC_API
plgd_time_status_t plgd_time_status(void);

/**
 * @brief Load persistent data of the plgd time resource from storage.
 *
 * @return true on success, data was loaded
 * @return false otherwise
 *
 * @note on successful call the plgd time status is set to
 * PLGD_TIME_STATUS_IN_SYNC_FROM_STORAGE
 */
OC_API
bool plgd_time_load(void);

/**
 * @brief Save persistent data of the plgd time resource to storage.
 *
 * @return true on success
 * @return false on failure
 */
OC_API
bool plgd_time_dump(void);

#ifdef OC_CLIENT

/**
 * @brief Callback invoked when plgd_time_fetch request receives a response or
 * timeouts.
 *
 * @param code response code of the time fetch request
 * @param time time provided by the endpoint (only valid if \p code is
 * OC_STATUS_OK)
 * @param data custom user data
 */
typedef void (*plgd_time_on_fetch_fn_t)(oc_status_t code, oc_clock_time_t time,
                                        void *data);

#if defined(OC_SECURITY) && defined(OC_PKI)

typedef struct
{
  oc_pki_verify_certificate_cb_t
    verify; ///< callback for certificate verification, if NULL then
            ///< it will be filled by a default function based on the value of
            ///< disable_time_verification
  oc_pki_user_data_t verify_data; ///< user data for the verify callback,
                                  ///< ignored if verify callback is NULL
  bool disable_time_verification; ///< ignore time validity checks when creating
                                  ///< a new TLS session for plgd-time fetch,
                                  ///< ignored if verify callback is not NULL
} plgd_time_fetch_verification_config_t;

#endif /* OC_SECURITY || OC_PKI */

typedef struct
{
  const oc_endpoint_t
    *endpoint;     ///< endpoint to fetch the time (cannot be NULL)
  const char *uri; ///< uri of the resource (cannot be NULL)
  plgd_time_on_fetch_fn_t on_fetch; ///< callback invoked when request finishes
                                    ///< or timeouts (cannot be NULL)
  void *on_fetch_data; ///< custom user data passed to the on_fetch callback

  uint16_t timeout; ///< timeout in seconds of the fetch time request (default
                    ///< value defined by PLGD_TIME_FETCH_TIMEOUT is used if
                    ///< value is 0)
#if defined(OC_SECURITY) && defined(OC_PKI)
  plgd_time_fetch_verification_config_t verification; ///< certificate
  int
    selected_identity_credid; ///< identity certificate to use for a created
                              ///< TLS session (set to -1 to use any available)
#endif
} plgd_time_fetch_config_t;

/** Convenience wrapper to create plgd_time_fetch_config_t with default \
 * verification function */
OC_API
plgd_time_fetch_config_t plgd_time_fetch_config(
  const oc_endpoint_t *endpoint, const char *uri,
  plgd_time_on_fetch_fn_t on_fetch, void *on_fetch_data, uint16_t timeout,
  int selected_identity_credid, bool disable_time_verification)
  OC_NONNULL(1, 2, 3);

#if defined(OC_SECURITY) && defined(OC_PKI)

/** Convenience wrapper to create plgd_time_fetch_config_t with a custom
 * verification function */
OC_API
plgd_time_fetch_config_t plgd_time_fetch_config_with_custom_verification(
  const oc_endpoint_t *endpoint, const char *uri,
  plgd_time_on_fetch_fn_t on_fetch, void *on_fetch_data, uint16_t timeout,
  int selected_identity_credid, oc_pki_verify_certificate_cb_t verify,
  oc_pki_user_data_t verify_data) OC_NONNULL(1, 2, 3, 7);

#endif /* OC_SECURITY && OC_PKI */

#ifdef OC_TCP
typedef enum {
  PLGD_TIME_FETCH_FLAG_TCP_SESSION_OPENED =
    1 << 0, // TCP session was opened and scheduled to close by plgd_time_fetch
} plgd_time_fetch_flags_t;

#endif /* OC_TCP */

/**
 * @brief Fetch time from an endpoint by a GET request
 *
 * For TLS/DTLS/TCP communication the request will use an existing session or
 * a peer. If there is no existing session or peer then it will attempt to
 * create it and after the request is finished it will be closed. (If the
 * session or peer existed before the fetch time request it won't be closed.)
 *
 * The closing of TCP sessions in iotivity-lite is asynchronous and is
 * executed on the network thread. There are no guarantees about the timing.
 * The session might've be already closed before plgd_time_fetch returns or it
 * might be closed some time later. This might cause an issue if you try to
 * create a TCP connection to the same endpoint right away after calling
 * plgd_time_fetch. If the asynchronous closing of the previous session hasn't
 * finished yet then it will close your session, because in the current
 * implementation sessions are indexed by the endpoint address and not by some
 * unique IDs. You can avoid this problem by checking for
 * PLGD_TIME_FETCH_FLAG_TCP_SESSION_OPENED. The flag is appended to \p flags
 * when plgd_time_fetch opens a new TCP session. (When a new session is opened
 * by plgd_time_fetch it will always be scheduled to close before the function
 * returns). To check if a session is closed use oc_tcp_connection_state. To
 * wait for a session to close use oc_add_session_event_callback_v1, which
 * will invoke a custom callback when a session is disconnected.
 *
 * @param fetch fetch time configuration
 * @param[out] flags output flags
 * @return true on success
 * @return false on failure
 *
 * @see plgd_time_fetch_config_t
 * @see plgd_time_fetch_flags_t
 * @see oc_add_session_event_callback_v1
 * @see oc_tcp_connection_state
 */
OC_API
bool plgd_time_fetch(plgd_time_fetch_config_t fetch, unsigned *flags);

#endif /* OC_CLIENT */

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* OC_HAS_FEATURE_PLGD_TIME */

#endif /* PLGD_TIME_H */
