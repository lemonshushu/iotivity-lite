#include "oc_api.h"
#include "port/oc_clock.h"
#include "oc_log.h"
#include <inttypes.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <sys/time.h>

static pthread_mutex_t mutex;
static pthread_cond_t cv;

static bool quit = false;

static char *rt = NULL;

struct timeval tv_GET;

static int
app_init(void)
{
  int ret = oc_init_platform("FASTEN", NULL, NULL);
  ret |= oc_add_device("/oic/d", "oic.d.phone", "FASTEN Client", "ocf.1.0.0",
                       "ocf.res.1.0.0", NULL, NULL);
  return ret;
}

#define MAX_URI_LENGTH (30)
static char a_fasten[MAX_URI_LENGTH];
static oc_endpoint_t *server = NULL;

static void
get_response_handler(oc_client_response_t *data)
{
  (void)data;
  return;
}

static oc_discovery_flags_t
discovery(const char *anchor, const char *uri, oc_string_array_t types,
          oc_interface_mask_t iface_mask, const oc_endpoint_t *endpoint,
          oc_resource_properties_t bm, void *user_data)
{
  (void)anchor;
  (void)iface_mask;
  (void)user_data;
  (void)bm;
  size_t uri_len = strlen(uri);
  uri_len = (uri_len >= MAX_URI_LENGTH) ? MAX_URI_LENGTH - 1 : uri_len;

  for (size_t i = 0; i < oc_string_array_get_allocated_size(types); i++) {
    char *t = oc_string_array_get_item(types, i);
    if (strlen(t) == 10 && strncmp(t, rt, 10) == 0) {
      strncpy(a_fasten, uri, uri_len);
      a_fasten[uri_len] = '\0';

      const oc_endpoint_t *ep = endpoint;
      while (ep != NULL) {
        if (!server && (ep->flags & TCP)) {
          oc_endpoint_list_copy(&server, ep);
        }
        ep = ep->next;
      }

      if (!server) {
        OC_PRINTF("ERROR: No TCP endpoint found\n");
        fflush(stdout);
        return OC_STOP_DISCOVERY;
      }

      gettimeofday(&tv_GET, NULL);
      OC_PRINTF("[DEBUG] Issued GET request at %ld.%06ld\n", tv_GET.tv_sec,
                tv_GET.tv_usec);
      if (!oc_do_get(a_fasten, server, NULL, &get_response_handler, LOW_QOS,
                     NULL)) {
        OC_PRINTF("ERROR: Could not issue GET request\n");
      }

      return OC_STOP_DISCOVERY;
    }
  }
  return OC_CONTINUE_DISCOVERY;
}

static void
issue_requests(void)
{
  oc_do_ip_discovery(rt, &discovery, NULL);
}

static void
signal_event_loop(void)
{
  pthread_cond_signal(&cv);
}

static void
handle_signal(int signal)
{
  (void)signal;
  quit = true;
  signal_event_loop();
}

static bool
init(void)
{
  struct sigaction sa;
  sigfillset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = handle_signal;
  sigaction(SIGINT, &sa, NULL);

  int err = pthread_mutex_init(&mutex, NULL);
  if (err != 0) {
    OC_PRINTF("ERROR: pthread_mutex_init failed (error=%d)!\n", err);
    return false;
  }
  pthread_condattr_t attr;
  err = pthread_condattr_init(&attr);
  if (err != 0) {
    OC_PRINTF("ERROR: pthread_condattr_init failed (error=%d)!\n", err);
    pthread_mutex_destroy(&mutex);
    return false;
  }
  err = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
  if (err != 0) {
    OC_PRINTF("ERROR: pthread_condattr_setclock failed (error=%d)!\n", err);
    pthread_condattr_destroy(&attr);
    pthread_mutex_destroy(&mutex);
    return false;
  }
  err = pthread_cond_init(&cv, &attr);
  if (err != 0) {
    OC_PRINTF("ERROR: pthread_cond_init failed (error=%d)!\n", err);
    pthread_condattr_destroy(&attr);
    pthread_mutex_destroy(&mutex);
    return false;
  }
  pthread_condattr_destroy(&attr);
  return true;
}

static void
deinit(void)
{
  pthread_cond_destroy(&cv);
  pthread_mutex_destroy(&mutex);
}

static void
run_loop(void)
{
  oc_clock_time_t next_event_mt;
  while (!quit) {
    next_event_mt = oc_main_poll_v1();
    pthread_mutex_lock(&mutex);
    if (next_event_mt == 0) {
      pthread_cond_wait(&cv, &mutex);
    } else {
      struct timespec next_event = { 1, 0 };
      oc_clock_time_t next_event_cv;
      if (oc_clock_monotonic_time_to_posix(next_event_mt, CLOCK_MONOTONIC,
                                           &next_event_cv)) {
        next_event = oc_clock_time_to_timespec(next_event_cv);
      }
      pthread_cond_timedwait(&cv, &mutex, &next_event);
    }
    pthread_mutex_unlock(&mutex);
  }
}

int
main(int argc, char **argv)
{
#ifndef OC_TCP
  OC_PRINTF("ERROR: This app requires TCP to be enabled\n");
  return 0;
#endif

  if (argc < 2) {
    OC_PRINTF("Usage: fasten_client <resource-type>\n");
    return -1;
  }

  rt = argv[1];

  if (!init()) {
    return -1;
  }

  static const oc_handler_t handler = {
    .init = app_init,
    .signal_event_loop = signal_event_loop,
    .requests_entry = issue_requests,
  };

  oc_storage_config("./fasten_client_creds");

  int ret = oc_main_init(&handler);
  if (ret < 0) {
    deinit();
    return ret;
  }
  run_loop();
  oc_main_shutdown();
  oc_free_server_endpoints(server);
  deinit();
  return 0;
}
