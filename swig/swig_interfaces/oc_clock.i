/* File oc_clock.i */
%module OCClock
%include "stdint.i"
#define OC_DYNAMIC_ALLOCATION

#if defined(__linux__) || defined(__ANDROID__)
#define CLOCKS_PER_SEC (1000000)
#endif

%ignore oc_clock_time_t;
typedef long long oc_clock_time_t;

%include "oc_config.h"

%pragma(java) jniclasscode=%{
  static {
    try {
        System.loadLibrary("iotivity-lite-jni");
    } catch (UnsatisfiedLinkError e) {
      System.err.println("Native code library failed to load. \n" + e);
      System.exit(1);
    }
  }
%}

%{
#include "port/oc_clock.h"
%}

%rename(clockInit) oc_clock_init;
%rename(clockTime) oc_clock_time;
%rename(clockTimeMonotonic) oc_clock_time_monotonic;
%rename(clockSeconds) oc_clock_seconds;
%rename(clockSecondsV1) oc_clock_seconds_v1;
%rename(clockWait) oc_clock_wait;

#define OC_API
#define OC_DEPRECATED(...)
#define OC_NONNULL(...)
%include "port/oc_clock.h"
