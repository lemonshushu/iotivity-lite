#include "oc_api.h"
#include "oc_log.h"
#include <pthread.h>
#include <signal.h>
#include <stdio.h>

static pthread_mutex_t mutex;
static pthread_cond_t cv;

static bool quit = false;

const uint8_t *data_64;
const uint8_t *data_128;
const uint8_t *data_256;
const uint8_t *data_512;
const uint8_t *data_1K;
const uint8_t *data_2K;
const uint8_t *data_4K;
const uint8_t *data_8K;
const uint8_t *data_16K;

static int
app_init(void)
{
  int ret = oc_init_platform("FASTEN", NULL, NULL);
  ret |= oc_add_device("/oic/d", "oic.d.fasten", "FASTEN Server", "ocf.1.0.0",
                       "ocf.res.1.0.0", NULL, NULL);
  return ret;
}

static void
get_request_handler_64(oc_request_t *request, oc_interface_mask_t interfaces,
                       void *user_data)
{
  (void)interfaces;
  (void)user_data;
  oc_rep_start_root_object();
  oc_rep_set_byte_string(root, data, data_64, 64);
  oc_rep_end_root_object();
  oc_send_response(request, OC_STATUS_OK);
}

static void
get_request_handler_128(oc_request_t *request, oc_interface_mask_t interfaces,
                        void *user_data)
{
  (void)interfaces;
  (void)user_data;
  oc_rep_start_root_object();
  oc_rep_set_byte_string(root, data, data_128, 128);
  oc_rep_end_root_object();
  oc_send_response(request, OC_STATUS_OK);
}

static void
get_request_handler_256(oc_request_t *request, oc_interface_mask_t interfaces,
                        void *user_data)
{
  (void)interfaces;
  (void)user_data;
  oc_rep_start_root_object();
  oc_rep_set_byte_string(root, data, data_256, 256);
  oc_rep_end_root_object();
  oc_send_response(request, OC_STATUS_OK);
}

static void
get_request_handler_512(oc_request_t *request, oc_interface_mask_t interfaces,
                        void *user_data)
{
  (void)interfaces;
  (void)user_data;
  oc_rep_start_root_object();
  oc_rep_set_byte_string(root, data, data_512, 512);
  oc_rep_end_root_object();
  oc_send_response(request, OC_STATUS_OK);
}

static void
get_request_handler_1K(oc_request_t *request, oc_interface_mask_t interfaces,
                       void *user_data)
{
  (void)interfaces;
  (void)user_data;
  oc_rep_start_root_object();
  oc_rep_set_byte_string(root, data, data_1K, 1024);
  oc_rep_end_root_object();
  oc_send_response(request, OC_STATUS_OK);
}

static void
get_request_handler_2K(oc_request_t *request, oc_interface_mask_t interfaces,
                       void *user_data)
{
  (void)interfaces;
  (void)user_data;
  oc_rep_start_root_object();
  oc_rep_set_byte_string(root, data, data_2K, 2048);
  oc_rep_end_root_object();
  oc_send_response(request, OC_STATUS_OK);
}

static void
get_request_handler_4K(oc_request_t *request, oc_interface_mask_t interfaces,
                       void *user_data)
{
  (void)interfaces;
  (void)user_data;
  oc_rep_start_root_object();
  oc_rep_set_byte_string(root, data, data_4K, 4096);
  oc_rep_end_root_object();
  oc_send_response(request, OC_STATUS_OK);
}

static void
get_request_handler_8K(oc_request_t *request, oc_interface_mask_t interfaces,
                       void *user_data)
{
  (void)interfaces;
  (void)user_data;
  oc_rep_start_root_object();
  oc_rep_set_byte_string(root, data, data_8K, 8192);
  oc_rep_end_root_object();
  oc_send_response(request, OC_STATUS_OK);
}

static void
get_request_handler_16K(oc_request_t *request, oc_interface_mask_t interfaces,
                        void *user_data)
{
  (void)interfaces;
  (void)user_data;
  oc_rep_start_root_object();
  oc_rep_set_byte_string(root, data, data_16K, 16384);
  oc_rep_end_root_object();
  oc_send_response(request, OC_STATUS_OK);
}

static void
_register_resources(oc_resource_t *res)
{
  oc_resource_bind_resource_interface(res, OC_IF_R);
  oc_resource_set_default_interface(res, OC_IF_R);
  oc_resource_set_discoverable(res, true);
  oc_resource_set_periodic_observable(res, 1);
}

static void
register_resources(void)
{
  oc_resource_t *res_64 = oc_new_resource("FASTEN_64", "/fasten_64", 1, 0);
  oc_resource_bind_resource_type(res_64, "fasten_64");
  _register_resources(res_64);
  oc_resource_set_request_handler(res_64, OC_GET, get_request_handler_64, NULL);
  oc_add_resource(res_64);

  oc_resource_t *res_128 = oc_new_resource("FASTEN_128", "/fasten_128", 1, 0);
  oc_resource_bind_resource_type(res_128, "fasten_128");
  _register_resources(res_128);
  oc_resource_set_request_handler(res_128, OC_GET, get_request_handler_128,
                                  NULL);
  oc_add_resource(res_128);

  oc_resource_t *res_256 = oc_new_resource("FASTEN_256", "/fasten_256", 1, 0);
  oc_resource_bind_resource_type(res_256, "fasten_256");
  _register_resources(res_256);
  oc_resource_set_request_handler(res_256, OC_GET, get_request_handler_256,
                                  NULL);
  oc_add_resource(res_256);

  oc_resource_t *res_512 = oc_new_resource("FASTEN_512", "/fasten_512", 1, 0);
  oc_resource_bind_resource_type(res_512, "fasten_512");
  _register_resources(res_512);
  oc_resource_set_request_handler(res_512, OC_GET, get_request_handler_512,
                                  NULL);
  oc_add_resource(res_512);

  oc_resource_t *res_1K = oc_new_resource("FASTEN_1K", "/fasten_1k", 1, 0);
  oc_resource_bind_resource_type(res_1K, "fasten_1k");
  _register_resources(res_1K);
  oc_resource_set_request_handler(res_1K, OC_GET, get_request_handler_1K, NULL);
  oc_add_resource(res_1K);

  oc_resource_t *res_2K = oc_new_resource("FASTEN_2K", "/fasten_2k", 1, 0);
  oc_resource_bind_resource_type(res_2K, "fasten_2k");
  _register_resources(res_2K);
  oc_resource_set_request_handler(res_2K, OC_GET, get_request_handler_2K, NULL);
  oc_add_resource(res_2K);

  oc_resource_t *res_4K = oc_new_resource("FASTEN_4K", "/fasten_4k", 1, 0);
  oc_resource_bind_resource_type(res_4K, "fasten_4k");
  _register_resources(res_4K);
  oc_resource_set_request_handler(res_4K, OC_GET, get_request_handler_4K, NULL);
  oc_add_resource(res_4K);

  oc_resource_t *res_8K = oc_new_resource("FASTEN_8K", "/fasten_8k", 1, 0);
  oc_resource_bind_resource_type(res_8K, "fasten_8k");
  _register_resources(res_8K);
  oc_resource_set_request_handler(res_8K, OC_GET, get_request_handler_8K, NULL);
  oc_add_resource(res_8K);

  oc_resource_t *res_16K = oc_new_resource("FASTEN_16K", "/fasten_16k", 1, 0);
  oc_resource_bind_resource_type(res_16K, "fasten_16k");
  _register_resources(res_16K);
  oc_resource_set_request_handler(res_16K, OC_GET, get_request_handler_16K,
                                  NULL);
  oc_add_resource(res_16K);
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

  data_64 = (uint8_t *)malloc(64);
  data_128 = (uint8_t *)malloc(128);
  data_256 = (uint8_t *)malloc(256);
  data_512 = (uint8_t *)malloc(512);
  data_1K = (uint8_t *)malloc(1024);
  data_2K = (uint8_t *)malloc(2048);
  data_4K = (uint8_t *)malloc(4096);
  data_8K = (uint8_t *)malloc(8192);
  data_16K = (uint8_t *)malloc(16384);
  memset((void *)data_64, 'A', 64);
  memset((void *)data_128, 'A', 128);
  memset((void *)data_256, 'A', 256);
  memset((void *)data_512, 'A', 512);
  memset((void *)data_1K, 'A', 1024);
  memset((void *)data_2K, 'A', 2048);
  memset((void *)data_4K, 'A', 4096);
  memset((void *)data_8K, 'A', 8192);
  memset((void *)data_16K, 'A', 16384);

  return true;
}

static void
deinit(void)
{
  pthread_cond_destroy(&cv);
  pthread_mutex_destroy(&mutex);

  free((void *)data_64);
  free((void *)data_128);
  free((void *)data_256);
  free((void *)data_512);
  free((void *)data_1K);
  free((void *)data_2K);
  free((void *)data_4K);
  free((void *)data_8K);
  free((void *)data_16K);
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