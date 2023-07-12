/****************************************************************************
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
 ***************************************************************************/

#include "api/oc_helpers_internal.h"
#include "messaging/coap/observe.h"
#include "messaging/coap/transactions.h"
#include "oc_core_res.h"
#include "oc_ri.h"
#include "port/oc_network_event_handler_internal.h"
#include "port/oc_random.h"
#include "tests/gtest/Device.h"
#include "tests/gtest/Endpoint.h"
#include "util/oc_list.h"

#include <array>
#include <gtest/gtest.h>
#include <string>

class TestObserver : public testing::Test {
public:
  static void SetUpTestCase()
  {
    oc_network_event_handler_mutex_init();
    oc_random_init();
  }

  static void TearDownTestCase()
  {
    coap_free_all_transactions(); // remove transactions generated by
                                  // coap_remove_observers_by_resource
    oc_random_destroy();
    oc_network_event_handler_mutex_destroy();
  }
};

TEST_F(TestObserver, AddObserver)
{
  std::string uri = "/res1";
  oc_resource_t res{};
  res.uri = oc_string_t{ nullptr, uri.length() + 1, &uri[0] };
  oc_endpoint_t ep =
    oc::endpoint::FromString("coap://[0:85a3::1319:8a2e:0:0]:1337");
  std::array<uint8_t, COAP_TOKEN_LEN> token;
  oc_random_buffer(token.data(), token.size());
  constexpr auto iface_mask =
    static_cast<oc_interface_mask_t>(OC_IF_BASELINE | OC_IF_RW);
  EXPECT_NE(nullptr, coap_add_observer(&res, 0, &ep, token.data(), token.size(),
                                       uri.c_str(), uri.length(), iface_mask));
  EXPECT_EQ(1, oc_list_length(coap_get_observers()));

  const coap_observer_t *observer =
    (coap_observer_t *)oc_list_head(coap_get_observers());
  EXPECT_EQ(1, res.num_observers);
  EXPECT_STREQ(uri.c_str(), oc_string(observer->url));
  EXPECT_EQ(0, oc_endpoint_compare(&ep, &observer->endpoint));
  EXPECT_EQ(0, memcmp(token.data(), &observer->token, token.size()));
  EXPECT_EQ(iface_mask, observer->iface_mask);
  EXPECT_EQ(&res, observer->resource);

  // must be called here so pointers to local variables are still valid
  coap_free_all_observers();
}

TEST_F(TestObserver, AddObserver_RemoveDuplicates)
{
  std::string uri = "/res1";
  oc_resource_t res{};
  res.uri = oc_string_t{ nullptr, uri.length() + 1, &uri[0] };
  constexpr uint16_t block_size = 1024;
  oc_endpoint_t ep =
    oc::endpoint::FromString("coap://[0:85a3::1319:8a2e:0:0]:1337");
  std::array<uint8_t, COAP_TOKEN_LEN> token;
  oc_random_buffer(token.data(), token.size());
  constexpr auto iface_mask =
    static_cast<oc_interface_mask_t>(OC_IF_BASELINE | OC_IF_RW);
  EXPECT_NE(nullptr,
            coap_add_observer(&res, block_size, &ep, token.data(), token.size(),
                              uri.c_str(), uri.length(), iface_mask));
  EXPECT_EQ(1, oc_list_length(coap_get_observers()));

  // duplicate -> same endpoint, url and iface_mask
  // - different resource doesn't matter
  std::string uri2 = "/res2";
  oc_resource_t res2{};
  res2.uri = oc_string_t{ nullptr, uri2.length() + 1, &uri2[0] };
  EXPECT_NE(nullptr,
            coap_add_observer(&res, block_size, &ep, token.data(), token.size(),
                              uri.c_str(), uri.length(), iface_mask));
  EXPECT_EQ(1, oc_list_length(coap_get_observers()));

  // - different block_size doesn't matter
  constexpr uint16_t block_size2 = 512;
  EXPECT_NE(nullptr, coap_add_observer(&res, block_size2, &ep, token.data(),
                                       token.size(), uri.c_str(), uri.length(),
                                       iface_mask));
  EXPECT_EQ(1, oc_list_length(coap_get_observers()));

  // - different token doesn't matter
  std::array<uint8_t, COAP_TOKEN_LEN> token2;
  oc_random_buffer(token2.data(), token2.size());
  EXPECT_NE(nullptr, coap_add_observer(&res, block_size, &ep, token2.data(),
                                       token2.size(), uri.c_str(), uri.length(),
                                       iface_mask));
  EXPECT_EQ(1, oc_list_length(coap_get_observers()));

  // must be called here so pointers to local variables are still valid
  coap_free_all_observers();
}

TEST_F(TestObserver, AddObserver_Multiples)
{
  std::string uri = "/res1";
  oc_resource_t res{};
  res.uri = oc_string_t{ nullptr, uri.length() + 1, &uri[0] };
  oc_endpoint_t ep =
    oc::endpoint::FromString("coap://[0:85a3::1319:8a2e:0:0]:1337");
  std::array<uint8_t, COAP_TOKEN_LEN> token;
  oc_random_buffer(token.data(), token.size());
  constexpr auto iface_mask =
    static_cast<oc_interface_mask_t>(OC_IF_BASELINE | OC_IF_RW);
  EXPECT_NE(nullptr, coap_add_observer(&res, 0, &ep, token.data(), token.size(),
                                       uri.c_str(), uri.length(), iface_mask));
  EXPECT_EQ(1, oc_list_length(coap_get_observers()));

  // different endpoint
  oc_endpoint_t ep2 =
    oc::endpoint::FromString("coap://[0:85a3::1319:8a2e:0:1]:1337");
  EXPECT_NE(nullptr,
            coap_add_observer(&res, 0, &ep2, token.data(), token.size(),
                              uri.c_str(), uri.length(), iface_mask));
  EXPECT_EQ(2, oc_list_length(coap_get_observers()));

  // different url
  std::string uri2 = "/res2";
  EXPECT_NE(nullptr,
            coap_add_observer(&res, 0, &ep, token.data(), token.size(),
                              uri2.c_str(), uri2.length(), iface_mask));
  EXPECT_EQ(3, oc_list_length(coap_get_observers()));

  // different iface_mask
  constexpr auto iface_mask2 = OC_IF_BASELINE;
  EXPECT_NE(nullptr, coap_add_observer(&res, 0, &ep, token.data(), token.size(),
                                       uri.c_str(), uri.length(), iface_mask2));
  EXPECT_EQ(4, oc_list_length(coap_get_observers()));

  // must be called here so pointers to local variables are still valid
  coap_free_all_observers();
}

#ifndef OC_DYNAMIC_ALLOCATION

TEST_F(TestObserver, AddObserver_Fail)
{
  std::string res_uri = "/res";
  oc_resource_t res{};
  res.uri = oc_string_t{ nullptr, res_uri.length() + 1, &res_uri[0] };
  oc_endpoint_t ep =
    oc::endpoint::FromString("coap://[0:85a3::1319:8a2e:0:0]:1337");
  std::array<uint8_t, COAP_TOKEN_LEN> token;
  oc_random_buffer(token.data(), token.size());
  constexpr auto iface_mask =
    static_cast<oc_interface_mask_t>(OC_IF_BASELINE | OC_IF_RW);

  for (size_t i = 0; i < COAP_MAX_OBSERVERS; ++i) {
    std::string uri = "/res/" + std::to_string(i);
    EXPECT_NE(nullptr,
              coap_add_observer(&res, 0, &ep, token.data(), token.size(),
                                uri.c_str(), uri.length(), iface_mask));
    EXPECT_EQ(i + 1, oc_list_length(coap_get_observers()));
  }

  ASSERT_EQ(COAP_MAX_OBSERVERS, oc_list_length(coap_get_observers()));
  // out of static memory
  EXPECT_EQ(nullptr,
            coap_add_observer(&res, 0, &ep, token.data(), token.size(),
                              res_uri.c_str(), res_uri.length(), iface_mask));

  // nothing was added
  EXPECT_EQ(COAP_MAX_OBSERVERS, oc_list_length(coap_get_observers()));

  // must be called here so pointers to local variables are still valid
  coap_free_all_observers();
}

#endif /* !OC_DYNAMIC_ALLOCATION */

TEST_F(TestObserver, RemoveAllByClient)
{
  std::string uri = "/res1";
  oc_resource_t res{};
  res.uri = oc_string_t{ nullptr, uri.length() + 1, &uri[0] };
  oc_endpoint_t ep =
    oc::endpoint::FromString("coap://[0:85a3::1319:8a2e:0:0]:1337");
  std::array<uint8_t, COAP_TOKEN_LEN> token;
  oc_random_buffer(token.data(), token.size());
  constexpr auto iface_mask =
    static_cast<oc_interface_mask_t>(OC_IF_BASELINE | OC_IF_RW);
  EXPECT_NE(nullptr, coap_add_observer(&res, 0, &ep, token.data(), token.size(),
                                       uri.c_str(), uri.length(), iface_mask));

  // different endpoint
  oc_endpoint_t ep2 =
    oc::endpoint::FromString("coap://[0:85a3::1319:8a2e:0:1]:1337");
  EXPECT_NE(nullptr,
            coap_add_observer(&res, 0, &ep2, token.data(), token.size(),
                              uri.c_str(), uri.length(), iface_mask));

  // different url
  std::string uri2 = "/res2";
  EXPECT_NE(nullptr,
            coap_add_observer(&res, 0, &ep, token.data(), token.size(),
                              uri2.c_str(), uri2.length(), iface_mask));

  // different iface_mask
  constexpr auto iface_mask2 = OC_IF_BASELINE;
  EXPECT_NE(nullptr, coap_add_observer(&res, 0, &ep, token.data(), token.size(),
                                       uri.c_str(), uri.length(), iface_mask2));
  ASSERT_EQ(4, oc_list_length(coap_get_observers()));

  // remove ep2 observers
  EXPECT_EQ(1, coap_remove_observers_by_client(&ep2));
  EXPECT_EQ(3, oc_list_length(coap_get_observers()));
  EXPECT_EQ(0, coap_remove_observers_by_client(&ep2));

  // remove ep1 observers
  EXPECT_EQ(3, coap_remove_observers_by_client(&ep));
  EXPECT_EQ(0, oc_list_length(coap_get_observers()));
  EXPECT_EQ(0, coap_remove_observers_by_client(&ep));
}

TEST_F(TestObserver, RemoveByToken)
{
  std::string uri = "/res";
  oc_resource_t res{};
  res.uri = oc_string_t{ nullptr, uri.length() + 1, &uri[0] };
  oc_endpoint_t ep1 =
    oc::endpoint::FromString("coap://[0:85a3::1319:8a2e:0:0]:1337");
  std::array<uint8_t, COAP_TOKEN_LEN> token1;
  oc_random_buffer(token1.data(), token1.size());
  constexpr auto iface_mask =
    static_cast<oc_interface_mask_t>(OC_IF_BASELINE | OC_IF_RW);
  // 1: token1 + ep1
  EXPECT_NE(nullptr,
            coap_add_observer(&res, 0, &ep1, token1.data(), token1.size(),
                              uri.c_str(), uri.length(), iface_mask));

  // 2: token1 + ep2
  oc_endpoint_t ep2 =
    oc::endpoint::FromString("coap://[0:85a3::1319:8a2e:0:1]:1337");
  EXPECT_NE(nullptr,
            coap_add_observer(&res, 0, &ep2, token1.data(), token1.size(),
                              uri.c_str(), uri.length(), iface_mask));

  // 3: token1 + ep1, different url
  std::string uri2 = "/res2";
  EXPECT_NE(nullptr,
            coap_add_observer(&res, 0, &ep1, token1.data(), token1.size(),
                              uri2.c_str(), uri2.length(), iface_mask));

  std::array<uint8_t, COAP_TOKEN_LEN> token2;
  oc_random_buffer(token2.data(), token2.size());
  // 4: token2 + ep1, different iface_mask
  constexpr auto iface_mask2 = OC_IF_BASELINE;
  EXPECT_NE(nullptr,
            coap_add_observer(&res, 0, &ep1, token2.data(), token2.size(),
                              uri.c_str(), uri.length(), iface_mask2));
  ASSERT_EQ(4, oc_list_length(coap_get_observers()));

  // remove token2 + ep2 observers
  EXPECT_FALSE(
    coap_remove_observer_by_token(&ep2, token2.data(), token2.size()));

  // remove token1 + ep1 observers - cases 1 and 3
  EXPECT_TRUE(
    coap_remove_observer_by_token(&ep1, token1.data(), token1.size()));
  EXPECT_TRUE(
    coap_remove_observer_by_token(&ep1, token1.data(), token1.size()));
  EXPECT_EQ(2, oc_list_length(coap_get_observers()));
  EXPECT_FALSE(
    coap_remove_observer_by_token(&ep1, token1.data(), token1.size()));

  // remove token1 + ep2 observers - case 2
  EXPECT_TRUE(
    coap_remove_observer_by_token(&ep2, token1.data(), token1.size()));
  EXPECT_EQ(1, oc_list_length(coap_get_observers()));
  EXPECT_FALSE(
    coap_remove_observer_by_token(&ep2, token1.data(), token1.size()));

  // remove token2 + ep1 observers - case 4
  EXPECT_TRUE(
    coap_remove_observer_by_token(&ep1, token2.data(), token2.size()));
  EXPECT_EQ(0, oc_list_length(coap_get_observers()));
  EXPECT_FALSE(
    coap_remove_observer_by_token(&ep1, token2.data(), token2.size()));
}

TEST_F(TestObserver, RemoveByMID)
{
  std::string uri = "/res";
  oc_resource_t res{};
  res.uri = oc_string_t{ nullptr, uri.length() + 1, &uri[0] };
  oc_endpoint_t ep1 =
    oc::endpoint::FromString("coap://[0:85a3::1319:8a2e:0:0]:1337");
  std::array<uint8_t, COAP_TOKEN_LEN> token;
  oc_random_buffer(token.data(), token.size());
  constexpr auto iface_mask =
    static_cast<oc_interface_mask_t>(OC_IF_BASELINE | OC_IF_RW);

  constexpr uint16_t mid1 = 1234;
  // 1: mid1 + ep1
  auto *obs = coap_add_observer(&res, 0, &ep1, token.data(), token.size(),
                                uri.c_str(), uri.length(), iface_mask);
  EXPECT_NE(nullptr, obs);
  obs->last_mid = mid1;

  // 2: mid1 + ep2
  oc_endpoint_t ep2 =
    oc::endpoint::FromString("coap://[0:85a3::1319:8a2e:0:1]:1337");
  obs = coap_add_observer(&res, 0, &ep2, token.data(), token.size(),
                          uri.c_str(), uri.length(), iface_mask);
  EXPECT_NE(nullptr, obs);
  obs->last_mid = mid1;

  // 3: mid1 + ep1, different url
  std::string uri2 = "/res2";
  obs = coap_add_observer(&res, 0, &ep1, token.data(), token.size(),
                          uri2.c_str(), uri2.length(), iface_mask);
  EXPECT_NE(nullptr, obs);
  obs->last_mid = mid1;

  constexpr uint16_t mid2 = 5678;
  // 4: mid2 + ep1, different iface_mask
  constexpr auto iface_mask2 = OC_IF_BASELINE;
  obs = coap_add_observer(&res, 0, &ep1, token.data(), token.size(),
                          uri.c_str(), uri.length(), iface_mask2);
  EXPECT_NE(nullptr, obs);
  obs->last_mid = mid2;

  ASSERT_EQ(4, oc_list_length(coap_get_observers()));

  // remove mid2 + ep2 observers
  EXPECT_FALSE(coap_remove_observer_by_mid(&ep2, mid2));
  EXPECT_EQ(4, oc_list_length(coap_get_observers()));

  // remove mid1 + ep1 observers - cases 1 and 3
  EXPECT_TRUE(coap_remove_observer_by_mid(&ep1, mid1));
  EXPECT_TRUE(coap_remove_observer_by_mid(&ep1, mid1));
  EXPECT_EQ(2, oc_list_length(coap_get_observers()));
  EXPECT_FALSE(coap_remove_observer_by_mid(&ep1, mid1));

  // remove mid1 + ep2 observers - case 2
  EXPECT_TRUE(coap_remove_observer_by_mid(&ep2, mid1));
  EXPECT_EQ(1, oc_list_length(coap_get_observers()));
  EXPECT_FALSE(coap_remove_observer_by_mid(&ep2, mid1));

  // remove mid2 + ep1 observers - case 4
  EXPECT_TRUE(coap_remove_observer_by_mid(&ep1, mid2));
  EXPECT_EQ(0, oc_list_length(coap_get_observers()));
  EXPECT_FALSE(coap_remove_observer_by_mid(&ep1, mid2));
}

TEST_F(TestObserver, RemoveAllObserversByResource)
{
  std::string uri1 = "/res/1";
  oc_resource_t res1{};
  res1.uri = oc_string_t{ nullptr, uri1.length() + 1, &uri1[0] };
  oc_endpoint_t ep1 =
    oc::endpoint::FromString("coap://[0:85a3::1319:8a2e:0:0]:1337");
  std::array<uint8_t, COAP_TOKEN_LEN> token;
  oc_random_buffer(token.data(), token.size());
  constexpr auto iface_mask =
    static_cast<oc_interface_mask_t>(OC_IF_BASELINE | OC_IF_RW);

  // 1: res1 + uri1 + ep1
  EXPECT_NE(nullptr,
            coap_add_observer(&res1, 0, &ep1, token.data(), token.size(),
                              uri1.c_str(), uri1.length(), iface_mask));

  // 2: res1 + uri1 + ep2
  oc_endpoint_t ep2 =
    oc::endpoint::FromString("coap://[0:85a3::1319:8a2e:0:1]:1337");
  EXPECT_NE(nullptr,
            coap_add_observer(&res1, 0, &ep2, token.data(), token.size(),
                              uri1.c_str(), uri1.length(), iface_mask));

  // 3: res1 + uri2 + ep1
  std::string uri2 = "/res/2";
  auto iface_mask2 = OC_IF_BASELINE;
  EXPECT_NE(nullptr,
            coap_add_observer(&res1, 0, &ep1, token.data(), token.size(),
                              uri2.c_str(), uri2.length(), iface_mask2));

  // 4: res2 + uri1 + ep1
  oc_resource_t res2{};
  res2.uri = oc_string_t{ nullptr, uri2.length() + 1, &uri2[0] };
  EXPECT_NE(nullptr,
            coap_add_observer(&res2, 0, &ep1, token.data(), token.size(),
                              uri1.c_str(), uri1.length(), iface_mask2));

  // 5: res2 + uri2 + ep1
  EXPECT_NE(nullptr,
            coap_add_observer(&res2, 0, &ep1, token.data(), token.size(),
                              uri2.c_str(), uri2.length(), iface_mask));

  // 6: res2 + uri2 + ep2
  EXPECT_NE(nullptr,
            coap_add_observer(&res2, 0, &ep2, token.data(), token.size(),
                              uri2.c_str(), uri2.length(), iface_mask));

  ASSERT_EQ(6, oc_list_length(coap_get_observers()));

  oc_resource_t res3{};
  res3.uri = oc_string_t{ nullptr, uri1.length() + 1, &uri1[0] };
  // remove all observers for res3
  EXPECT_EQ(0, coap_remove_observers_by_resource(&res3));

  oc_resource_t res4{};
  EXPECT_EQ(0, coap_remove_observers_by_resource(&res4));

  // remove all observers for res1 + uri1 - cases 1 and 2
  EXPECT_EQ(2, coap_remove_observers_by_resource(&res1));
  EXPECT_EQ(4, oc_list_length(coap_get_observers()));

  // remove all observers for res2 + uri2 - cases 5 and 6
  EXPECT_EQ(2, coap_remove_observers_by_resource(&res2));
  EXPECT_EQ(2, oc_list_length(coap_get_observers()));

  // must be called here so pointers to local variables are still valid
  coap_free_all_observers();
}

static constexpr size_t kDeviceID{ 0 };

class TestObserverWithServer : public testing::Test {
public:
  static void SetUpTestCase() { ASSERT_TRUE(oc::TestDevice::StartServer()); }

  static void TearDownTestCase() { oc::TestDevice::StopServer(); }
};

#ifdef OC_SECURITY

TEST_F(TestObserverWithServer, RemoveAllObserversOnDOSChange)
{
  const oc_endpoint_t *ep = oc::TestDevice::GetEndpoint(kDeviceID, 0, SECURED);
  ASSERT_NE(nullptr, ep);

  std::array<uint8_t, COAP_TOKEN_LEN> token;
  oc_random_buffer(token.data(), token.size());

  oc_resource_t *platform = oc_core_get_resource_by_index(OCF_P, kDeviceID);
  ASSERT_NE(nullptr, platform);
  EXPECT_NE(nullptr,
            coap_add_observer(platform, 1024, ep, token.data(), token.size(),
                              oc_string(platform->uri),
                              oc_string_len(platform->uri), OC_IF_BASELINE));

  oc_resource_t *con = oc_core_get_resource_by_index(OCF_CON, kDeviceID);
  ASSERT_NE(nullptr, con);
  EXPECT_NE(nullptr,
            coap_add_observer(con, 1024, ep, token.data(), token.size(),
                              oc_string(con->uri), oc_string_len(con->uri),
                              OC_IF_BASELINE));

  oc_resource_t *doxm = oc_core_get_resource_by_index(OCF_SEC_DOXM, kDeviceID);
  ASSERT_NE(nullptr, doxm);
  EXPECT_NE(nullptr,
            coap_add_observer(doxm, 1024, ep, token.data(), token.size(),
                              oc_string(doxm->uri), oc_string_len(doxm->uri),
                              OC_IF_BASELINE));

  size_t kInvalidDeviceID = std::numeric_limits<size_t>::max();
  EXPECT_EQ(0, coap_remove_observers_on_dos_change(kInvalidDeviceID, false));

  // con observer removed
  EXPECT_EQ(1, coap_remove_observers_on_dos_change(kDeviceID, false));

  // platform and doxm observers removed
  EXPECT_EQ(2, coap_remove_observers_on_dos_change(kDeviceID, true));
}

#endif // OC_SECURITY
