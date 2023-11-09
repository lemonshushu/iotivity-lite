#include "oc_api.h"
#include "oc_log.h"
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#define MAX_PAYLOAD_SIZE (10485760)

static pthread_mutex_t mutex;
static pthread_cond_t cv;

static bool quit = false;

const uint8_t *fasten_data;

static int
app_init(void)
{
  int ret = oc_init_platform("FASTEN", NULL, NULL);
  ret |= oc_add_device("/oic/d", "oic.d.fasten", "FASTEN Server", "ocf.1.0.0",
                       "ocf.res.1.0.0", NULL, NULL);
  return ret;
}

static void
get_request_handler(oc_request_t *request, oc_interface_mask_t interfaces,
                    void *user_data)
{
  (void)interfaces;
  (void)user_data;
  size_t payload_size = 0;
  const char *query = request->query;
  if (strncmp(query, "size=", 5) == 0) {
    payload_size = (size_t)strtoul(query + 5, NULL, 10);
  }

  oc_rep_start_root_object();
  oc_rep_set_byte_string(root, data, fasten_data, payload_size);
  oc_rep_end_root_object();
  oc_send_response(request, OC_STATUS_OK);
}

static void
register_resources(void)
{
  oc_resource_t *res = oc_new_resource("FASTEN", "/fasten", 1, 0);
  oc_resource_bind_resource_type(res, "fasten");
  oc_resource_bind_resource_interface(res, OC_IF_R);
  oc_resource_set_default_interface(res, OC_IF_R);
  oc_resource_set_discoverable(res, true);
  oc_resource_set_periodic_observable(res, 1);
  oc_resource_set_request_handler(res, OC_GET, get_request_handler, NULL);
  oc_add_resource(res);
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

  fasten_data = (uint8_t *)malloc(MAX_PAYLOAD_SIZE);
  return true;
}

static void
deinit(void)
{
  pthread_cond_destroy(&cv);
  pthread_mutex_destroy(&mutex);

  free((void *)fasten_data);
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
main(void)
{
#ifndef OC_TCP
  OC_PRINTF("ERROR: This app requires TCP to be enabled\n");
  return 0;
#endif
  if (!init()) {
    return -1;
  }

  static const oc_handler_t handler = {
    .init = app_init,
    .signal_event_loop = signal_event_loop,
    .register_resources = register_resources,
  };

  oc_storage_config("./fasten_server_creds");

  int ret = oc_main_init(&handler);
  if (ret < 0) {
    deinit();
    return ret;
  }
  run_loop();
  oc_main_shutdown();
  deinit();
  return 0;
}
