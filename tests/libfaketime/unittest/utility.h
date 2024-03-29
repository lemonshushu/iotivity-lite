/******************************************************************
 *
 * Copyright (c) 2023 plgd.dev s.r.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"),
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#pragma once

#include "port/oc_clock.h"

#include <stdbool.h>
#include <chrono>

namespace oc {

inline oc_clock_time_t testStart;
inline oc_clock_time_t testStartMT;

bool SetSystemTime(oc_clock_time_t now, std::chrono::milliseconds shift);

#ifdef __unix__
bool SetSystemTimeUnix(oc_clock_time_t now, std::chrono::milliseconds shift);
#endif /* __unix__ */

#ifdef _WIN32
bool SetSystemTimeWin(oc_clock_time_t now, std::chrono::milliseconds shift);
#endif /* _WIN32 */

void SetTestStartTime();

bool RestoreSystemTimeFromTestStartTime();

} // namespace oc
