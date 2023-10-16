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

#include "api/oc_rep_encode_internal.h"
#include "api/oc_rep_decode_internal.h"
#include "oc_rep.h"
#include "port/oc_log_internal.h"
#include "tests/gtest/RepPool.h"

#include <gtest/gtest.h>
#include <limits>
#include <stdlib.h>
#include <string>
#include <vector>

static const oc_rep_encoder_type_t g_rep_default_encoder =
  oc_rep_encoder_get_type();
static const oc_rep_decoder_type_t g_rep_default_decoder =
  oc_rep_decoder_get_type();

class TestRepEncodeWithRealloc : public testing::Test {
public:
  static void SetUpTestCase()
  {
    oc_rep_encoder_set_type(OC_REP_CBOR_ENCODER);
    oc_rep_decoder_set_type(OC_REP_CBOR_DECODER);
  }

  static void TearDownTestCase()
  {
    oc_rep_encoder_set_type(g_rep_default_encoder);
    oc_rep_decoder_set_type(g_rep_default_decoder);
  }

  void TearDown() override
  {
#ifdef OC_DYNAMIC_ALLOCATION
    free(buffer_);
#endif /* OC_DYNAMIC_ALLOCATION */
  }

  /* buffer for oc_rep_t */
  void SetRepBuffer(size_t size = 1024, size_t max_size = 1024)
  {
#ifdef OC_DYNAMIC_ALLOCATION
    if (buffer_ != nullptr) {
      free(buffer_);
    }
    buffer_ = nullptr;
    if (size > 0) {
      buffer_ = static_cast<uint8_t *>(malloc(size));
    }
    oc_rep_new_realloc_v1(&buffer_, size, max_size);
#else  /* OC_DYNAMIC_ALLOCATION */
    (void)size;
    buffer_.resize(max_size);
    oc_rep_new_v1(buffer_.data(), buffer_.size());
    memset(rep_objects_alloc_, 0, OC_MAX_NUM_REP_OBJECTS * sizeof(char));
    memset(rep_objects_pool_, 0, OC_MAX_NUM_REP_OBJECTS * sizeof(oc_rep_t));
#endif /* OC_DYNAMIC_ALLOCATION */
  }

  void Shrink()
  {
#ifdef OC_DYNAMIC_ALLOCATION
    buffer_ = oc_rep_shrink_encoder_buf(buffer_);
#endif /* OC_DYNAMIC_ALLOCATION */
  }

  oc::oc_rep_unique_ptr ParsePayload()
  {
    const uint8_t *payload = oc_rep_get_encoder_buf();
    int payload_len = oc_rep_get_encoded_payload_size();
    EXPECT_NE(payload_len, -1);
    oc_rep_set_pool(&rep_objects_);
    oc_rep_t *rep = nullptr;
    EXPECT_EQ(CborNoError, oc_parse_rep(payload, payload_len, &rep));
    return oc::oc_rep_unique_ptr(rep, &oc_free_rep);
  }

#ifdef OC_DYNAMIC_ALLOCATION
  uint8_t *buffer_{ nullptr };
  oc_memb rep_objects_{ sizeof(oc_rep_t), 0, nullptr, nullptr, nullptr };
#else  /* !OC_DYNAMIC_ALLOCATION */
  std::vector<uint8_t> buffer_{};
  char rep_objects_alloc_[OC_MAX_NUM_REP_OBJECTS];
  oc_rep_t rep_objects_pool_[OC_MAX_NUM_REP_OBJECTS];
  oc_memb rep_objects_{ sizeof(oc_rep_t), OC_MAX_NUM_REP_OBJECTS,
                        rep_objects_alloc_, (void *)rep_objects_pool_,
                        nullptr };
#endif /* OC_DYNAMIC_ALLOCATION */
};

TEST_F(TestRepEncodeWithRealloc, OCRepEncodedPayloadRealloc)
{
  SetRepBuffer(1, 1024);

  oc_rep_start_root_object();
  ASSERT_EQ(CborNoError, oc_rep_get_cbor_errno());
  oc_rep_set_text_string(root, hello, "world");
  ASSERT_EQ(CborNoError, oc_rep_get_cbor_errno());
  oc_rep_set_double(root, double, 3.14);
  ASSERT_EQ(CborNoError, oc_rep_get_cbor_errno());
  oc_rep_set_boolean(root, bool, true);
  ASSERT_EQ(CborNoError, oc_rep_get_cbor_errno());
  oc_rep_set_int(root, int, -1);
  ASSERT_EQ(CborNoError, oc_rep_get_cbor_errno());
  oc_rep_set_uint(root, uint, -1);
  ASSERT_EQ(CborNoError, oc_rep_get_cbor_errno());
  std::vector<uint8_t> byte_string = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 };
  oc_rep_set_byte_string(root, byte_string_key, byte_string.data(),
                         byte_string.size());
  ASSERT_EQ(CborNoError, oc_rep_get_cbor_errno());
  std::vector<int> fib = { 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89 };
  oc_rep_set_key(oc_rep_object(root), "fibonacci");
  ASSERT_EQ(CborNoError, oc_rep_get_cbor_errno());
  oc_rep_begin_array(oc_rep_object(root), fibonacci);
  ASSERT_EQ(CborNoError, oc_rep_get_cbor_errno());
  for (const auto &val : fib) {
    oc_rep_add_int(fibonacci, val);
    ASSERT_EQ(CborNoError, oc_rep_get_cbor_errno());
  }
  oc_rep_end_array(oc_rep_object(root), fibonacci);
  ASSERT_EQ(CborNoError, oc_rep_get_cbor_errno());
  std::vector<double> math_constants = { 3.14159, 2.71828, 1.414121, 1.61803 };
  oc_rep_set_double_array(root, math_constants, math_constants.data(),
                          math_constants.size());
  ASSERT_EQ(CborNoError, oc_rep_get_cbor_errno());
  oc_rep_end_root_object();
  ASSERT_EQ(CborNoError, oc_rep_get_cbor_errno());

  auto rep = ParsePayload();
  ASSERT_NE(nullptr, rep.get());
  OC_DBG("payload: %s", oc::RepPool::GetJson(rep.get(), true).data());
  size_t payload_size = oc_rep_get_encoded_payload_size();
  EXPECT_EQ(156, payload_size);
#ifdef OC_DYNAMIC_ALLOCATION
  EXPECT_GT(oc_rep_get_encoder_buffer_size(), payload_size);
#endif /* OC_DYNAMIC_ALLOCATION */
  Shrink();
#ifdef OC_DYNAMIC_ALLOCATION
  EXPECT_EQ(oc_rep_get_encoder_buffer_size(), payload_size);
#endif /* OC_DYNAMIC_ALLOCATION */
}

TEST_F(TestRepEncodeWithRealloc, OCRepEncodeRaw)
{
  std::vector<uint8_t> in{ '\0' };
  SetRepBuffer(0, 0);
  oc_rep_encode_raw(in.data(), in.size());
  EXPECT_EQ(CborErrorInternalError, oc_rep_get_cbor_errno());

  SetRepBuffer(1, 1);
  oc_rep_encode_raw(in.data(), in.size());
  EXPECT_EQ(CborNoError, oc_rep_get_cbor_errno());
  oc_rep_encode_raw(in.data(), in.size());
  EXPECT_EQ(CborErrorOutOfMemory, oc_rep_get_cbor_errno());

  SetRepBuffer(1, 8);
  for (size_t i = 0; i < 8; ++i) {
    oc_rep_encode_raw(in.data(), in.size());
    EXPECT_EQ(CborNoError, oc_rep_get_cbor_errno());
  }
  EXPECT_EQ(8, oc_rep_get_encoded_payload_size());

  oc_rep_encode_raw(in.data(), in.size());
  EXPECT_EQ(CborErrorOutOfMemory, oc_rep_get_cbor_errno());
}

TEST_F(TestRepEncodeWithRealloc, OCRepEncodeNull)
{
  SetRepBuffer(1, 1);
  ASSERT_EQ(CborNoError, oc_rep_encode_null(oc_rep_get_encoder()));
  EXPECT_EQ(CborErrorOutOfMemory, oc_rep_encode_null(oc_rep_get_encoder()));

  SetRepBuffer(1, 8);
  for (size_t i = 0; i < 8; ++i) {
    ASSERT_EQ(CborNoError, oc_rep_encode_null(oc_rep_get_encoder()));
  }
  ASSERT_EQ(8, oc_rep_get_encoded_payload_size());
  EXPECT_EQ(CborErrorOutOfMemory, oc_rep_encode_null(oc_rep_get_encoder()));
}

TEST_F(TestRepEncodeWithRealloc, OCRepEncodeBool)
{
  SetRepBuffer(1, 1);
  ASSERT_EQ(CborNoError, oc_rep_encode_boolean(oc_rep_get_encoder(), false));
  EXPECT_EQ(CborErrorOutOfMemory,
            oc_rep_encode_boolean(oc_rep_get_encoder(), true));

  SetRepBuffer(1, 8);
  for (size_t i = 0; i < 8; ++i) {
    ASSERT_EQ(CborNoError,
              oc_rep_encode_boolean(oc_rep_get_encoder(), i % 2 == 0));
  }
  ASSERT_EQ(8, oc_rep_get_encoded_payload_size());
  EXPECT_EQ(CborErrorOutOfMemory,
            oc_rep_encode_boolean(oc_rep_get_encoder(), false));
}

TEST_F(TestRepEncodeWithRealloc, OCRepEncodeInt)
{
  SetRepBuffer(1, 1);
  EXPECT_EQ(CborErrorOutOfMemory,
            oc_rep_encode_int(oc_rep_get_encoder(), INT64_MAX));

  SetRepBuffer(1, 9);
  ASSERT_EQ(CborNoError, oc_rep_encode_int(oc_rep_get_encoder(), INT64_MAX));
  ASSERT_EQ(9, oc_rep_get_encoded_payload_size());
  EXPECT_EQ(CborErrorOutOfMemory,
            oc_rep_encode_int(oc_rep_get_encoder(), INT64_MAX));
}

TEST_F(TestRepEncodeWithRealloc, OCRepEncodeUint)
{
  SetRepBuffer(1, 1);
  EXPECT_EQ(CborErrorOutOfMemory,
            oc_rep_encode_uint(oc_rep_get_encoder(), UINT64_MAX));

  SetRepBuffer(1, 9);
  ASSERT_EQ(CborNoError, oc_rep_encode_uint(oc_rep_get_encoder(), UINT64_MAX));
  ASSERT_EQ(9, oc_rep_get_encoded_payload_size());
  EXPECT_EQ(CborErrorOutOfMemory,
            oc_rep_encode_uint(oc_rep_get_encoder(), UINT64_MAX));
}

TEST_F(TestRepEncodeWithRealloc, OCRepEncodeFloat)
{
  float val = 0;
  SetRepBuffer(1, 1);
  EXPECT_EQ(CborErrorOutOfMemory, oc_rep_encode_floating_point(
                                    oc_rep_get_encoder(), CborFloatType, &val));

  SetRepBuffer(1, 5);
  val = std::numeric_limits<float>::max();
  ASSERT_EQ(CborNoError, oc_rep_encode_floating_point(oc_rep_get_encoder(),
                                                      CborFloatType, &val));
  ASSERT_EQ(5, oc_rep_get_encoded_payload_size());
  EXPECT_EQ(CborErrorOutOfMemory, oc_rep_encode_floating_point(
                                    oc_rep_get_encoder(), CborFloatType, &val));
}

TEST_F(TestRepEncodeWithRealloc, OCRepEncodeDouble)
{
  SetRepBuffer(1, 1);
  EXPECT_EQ(CborErrorOutOfMemory,
            oc_rep_encode_double(oc_rep_get_encoder(),
                                 std::numeric_limits<double>::max()));

  SetRepBuffer(1, 9);
  ASSERT_EQ(CborNoError,
            oc_rep_encode_double(oc_rep_get_encoder(),
                                 std::numeric_limits<double>::max()));
  ASSERT_EQ(9, oc_rep_get_encoded_payload_size());
  EXPECT_EQ(CborErrorOutOfMemory,
            oc_rep_encode_double(oc_rep_get_encoder(),
                                 std::numeric_limits<double>::max()));
}

TEST_F(TestRepEncodeWithRealloc, OCRepEncodeTextString)
{
  SetRepBuffer(1, 1);
  std::string str = "test";
  EXPECT_EQ(
    CborErrorOutOfMemory,
    oc_rep_encode_text_string(oc_rep_get_encoder(), str.c_str(), str.length()));

  SetRepBuffer(1, 17);
  str = "this is 16 chars";
  ASSERT_EQ(CborNoError, oc_rep_encode_text_string(oc_rep_get_encoder(),
                                                   str.c_str(), str.length()));
  ASSERT_EQ(17, oc_rep_get_encoded_payload_size());

  str = "c";
  EXPECT_EQ(
    CborErrorOutOfMemory,
    oc_rep_encode_text_string(oc_rep_get_encoder(), str.c_str(), str.length()));
}

TEST_F(TestRepEncodeWithRealloc, OCRepEncodeByteString)
{
  SetRepBuffer(1, 1);
  std::vector<uint8_t> bstr = { 0x42, 0x0,  0x42, 0x0,  0x42, 0x0,  0x42, 0x42,
                                0x0,  0x42, 0x0,  0x42, 0x0,  0x42, 0x42, 0x0 };
  EXPECT_EQ(
    CborErrorOutOfMemory,
    oc_rep_encode_byte_string(oc_rep_get_encoder(), bstr.data(), bstr.size()));

  SetRepBuffer(1, 20);
  EXPECT_EQ(CborNoError, oc_rep_encode_byte_string(oc_rep_get_encoder(),
                                                   bstr.data(), bstr.size()));
}

TEST_F(TestRepEncodeWithRealloc, OCRepEncodeArray)
{
  SetRepBuffer(1, 1);
  CborEncoder array{};
  CborEncoder inner_array{};
  ASSERT_EQ(CborNoError, oc_rep_encoder_create_array(
                           oc_rep_get_encoder(), &array, CborIndefiniteLength));
  EXPECT_EQ(
    CborErrorOutOfMemory,
    oc_rep_encoder_create_array(&array, &inner_array, CborIndefiniteLength));

  SetRepBuffer(1, 1);
  array = {};
  ASSERT_EQ(CborNoError, oc_rep_encoder_create_array(
                           oc_rep_get_encoder(), &array, CborIndefiniteLength));
  EXPECT_EQ(CborErrorOutOfMemory,
            oc_rep_encoder_close_container(oc_rep_get_encoder(), &array));

  SetRepBuffer(1, 3);
  array = {};
  ASSERT_EQ(CborNoError, oc_rep_encoder_create_array(
                           oc_rep_get_encoder(), &array, CborIndefiniteLength));
  ASSERT_EQ(CborNoError, oc_rep_encode_boolean(&array, true));
  ASSERT_EQ(CborNoError,
            oc_rep_encoder_close_container(oc_rep_get_encoder(), &array));
  ASSERT_EQ(3, oc_rep_get_encoded_payload_size());
}

TEST_F(TestRepEncodeWithRealloc, OCRepEncodeMap)
{
  SetRepBuffer(1, 1);
  CborEncoder map{};
  CborEncoder inner_map{};
  ASSERT_EQ(CborNoError, oc_rep_encoder_create_map(oc_rep_get_encoder(), &map,
                                                   CborIndefiniteLength));
  EXPECT_EQ(CborErrorOutOfMemory,
            oc_rep_encoder_create_map(&map, &inner_map, CborIndefiniteLength));

  SetRepBuffer(1, 1);
  map = {};
  ASSERT_EQ(CborNoError, oc_rep_encoder_create_map(oc_rep_get_encoder(), &map,
                                                   CborIndefiniteLength));
  EXPECT_EQ(CborErrorOutOfMemory,
            oc_rep_encoder_close_container(oc_rep_get_encoder(), &map));

  std::string key = "key";
  SetRepBuffer(1, 7);
  map = {};
  ASSERT_EQ(CborNoError, oc_rep_encoder_create_map(oc_rep_get_encoder(), &map,
                                                   CborIndefiniteLength));
  ASSERT_EQ(CborNoError,
            oc_rep_encode_text_string(&map, key.c_str(), key.length()));
  ASSERT_EQ(CborNoError, oc_rep_encode_boolean(&map, true));
  ASSERT_EQ(CborNoError,
            oc_rep_encoder_close_container(oc_rep_get_encoder(), &map));
  ASSERT_EQ(7, oc_rep_get_encoded_payload_size());

  auto rep = ParsePayload();
  ASSERT_NE(nullptr, rep.get());
  OC_DBG("payload: %s", oc::RepPool::GetJson(rep.get(), true).data());
}
