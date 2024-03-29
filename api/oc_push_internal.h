/****************************************************************************
 *
 * Copyright 2021 ETRI All Rights Reserved.
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
 * Created on: Aug 23, 2021,
 * 				Author: Joo-Chul Kevin Lee (rune@etri.re.kr)
 *
 *
 ****************************************************************************/

#ifndef OC_PUSH_INTERNAL_H
#define OC_PUSH_INTERNAL_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void oc_push_init(void);

void oc_push_free(void);

void oc_create_pushconf_resource(size_t device_index);

void oc_create_pushreceiver_resource(size_t device_index);

#ifdef __cplusplus
}
#endif

#endif /* OC_PUSH_INTERNAL_H */
