/******************************************************************
 *
 * Copyright 2023 Daniel Adam, All Rights Reserved.
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

#if defined(OC_SECURITY) && defined(OC_PKI)

#include "oc_pki.h"
#include "oc_config.h"
#include "security/oc_pki_internal.h"
#include "tests/gtest/PKI.h"

#include <stdbool.h>
#include <gtest/gtest.h>

class TestPKIPK : public testing::Test {
public:
  void SetUp() override { oc::pki::PKDummyFunctions::Clear(); }

  void TearDown() override { oc_pki_set_pk_functions(nullptr); }
};

TEST_F(TestPKIPK, pk_functions)
{
  using namespace oc::pki;

  EXPECT_TRUE(oc_pki_set_pk_functions(nullptr));
  EXPECT_FALSE(oc_pki_get_pk_functions(nullptr));
  oc_pki_pk_functions_t pk_functions = PKDummyFunctions::GetPKFunctions();
  EXPECT_TRUE(oc_pki_set_pk_functions(&pk_functions));
  EXPECT_TRUE(oc_pki_get_pk_functions(nullptr));
  pk_functions.mbedtls_pk_parse_key = nullptr;
  EXPECT_FALSE(oc_pki_set_pk_functions(&pk_functions));
  pk_functions.mbedtls_pk_parse_key = PKDummyFunctions::ParseKey;
  pk_functions.mbedtls_pk_write_key_der = nullptr;
  EXPECT_FALSE(oc_pki_set_pk_functions(&pk_functions));
  pk_functions.mbedtls_pk_write_key_der = PKDummyFunctions::WriteKeyDer;
  pk_functions.mbedtls_pk_ecp_gen_key = nullptr;
  EXPECT_FALSE(oc_pki_set_pk_functions(&pk_functions));
  pk_functions.mbedtls_pk_ecp_gen_key = PKDummyFunctions::GenKey;
  pk_functions.pk_free_key = nullptr;
  EXPECT_FALSE(oc_pki_set_pk_functions(&pk_functions));

  oc_pki_pk_functions_t get_pk_functions{};
  EXPECT_TRUE(oc_pki_get_pk_functions(&get_pk_functions));
  EXPECT_EQ(get_pk_functions.mbedtls_pk_parse_key, &PKDummyFunctions::ParseKey);
  EXPECT_EQ(get_pk_functions.mbedtls_pk_write_key_der,
            &PKDummyFunctions::WriteKeyDer);
  EXPECT_EQ(get_pk_functions.mbedtls_pk_ecp_gen_key, &PKDummyFunctions::GenKey);
  EXPECT_EQ(get_pk_functions.pk_free_key, &PKDummyFunctions::FreeKey);
}

TEST_F(TestPKIPK, pk_free_key)
{
  oc_pki_pk_functions_t pk_functions =
    oc::pki::PKDummyFunctions::GetPKFunctions();
  EXPECT_FALSE(oc_pk_free_key(0, nullptr, 0));
  EXPECT_TRUE(oc_pki_set_pk_functions(&pk_functions));
  EXPECT_TRUE(oc_pk_free_key(0, nullptr, 0));
  EXPECT_TRUE(oc::pki::PKDummyFunctions::freeKeyInvoked);
}

TEST_F(TestPKIPK, pk_gen_key)
{
  oc_pki_pk_functions_t pk_functions =
    oc::pki::PKDummyFunctions::GetPKFunctions();
  EXPECT_TRUE(oc_pki_set_pk_functions(&pk_functions));
  oc_mbedtls_pk_ecp_gen_key(0, MBEDTLS_ECP_DP_SECP256R1, nullptr, nullptr,
                            nullptr);
  EXPECT_TRUE(oc::pki::PKDummyFunctions::genKeyInvoked);
}

TEST_F(TestPKIPK, pk_write_key_der)
{
  oc_pki_pk_functions_t pk_functions =
    oc::pki::PKDummyFunctions::GetPKFunctions();
  EXPECT_TRUE(oc_pki_set_pk_functions(&pk_functions));
  oc_mbedtls_pk_write_key_der(0, nullptr, nullptr, 0);
  EXPECT_TRUE(oc::pki::PKDummyFunctions::writeKeyDerInvoked);
}

TEST_F(TestPKIPK, pk_parse_key)
{
  oc_pki_pk_functions_t pk_functions =
    oc::pki::PKDummyFunctions::GetPKFunctions();
  EXPECT_TRUE(oc_pki_set_pk_functions(&pk_functions));
  oc_mbedtls_pk_parse_key(0, nullptr, nullptr, 0, nullptr, 0, nullptr, nullptr);
  EXPECT_TRUE(oc::pki::PKDummyFunctions::parseKeyInvoked);
}

#endif /* OC_SECURITY && OC_PKI */