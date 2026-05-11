/*MIT License

Copyright (c) 2023 Overwolf Ltd.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
#include "crypto_service_impl.h"
#include <common/container_utils.h>

#include <services/logger/logger_macros.h>

#include <JsonUtilities.h>
#include <Misc/Base64.h>

#ifdef CFCORE_WITH_OPENSSL

#define UI UI_ST
THIRD_PARTY_INCLUDES_START
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
//#include <openssl/err.h>
THIRD_PARTY_INCLUDES_END
#undef UI

#endif

namespace cfcore {

#ifdef CFCORE_WITH_OPENSSL

struct EVP_MD_CTX_Deleter {
  void operator()(EVP_MD_CTX* context) const {
    EVP_MD_CTX_destroy(context);
  }
};

struct BIO_Deleter {
  void operator()(BIO* bio) const {
    BIO_free(bio);
  }
};

#endif
}

using namespace cfcore;

// -----------------------------------------------------------------------------
// NOTE(twolf): This doesn't support unicode for any of the strings - mainly
// where we call EVP_DigestVerifyUpdate
bool CryptoServiceImpl::DigestVerifyPS256(const FString& public_key_pem,
                                          const FString& signature,
                                          const FString& payload) {

#ifdef CFCORE_WITH_OPENSSL
  TArray<uint8> signature_buff;
  bool decoded = FBase64::Decode(signature, signature_buff);

  if (!decoded) {
    CFCORE_LOG(Error, TEXT("Failed decoding base64 signature"));
    return false;
  }

  TUniquePtr<EVP_MD_CTX, EVP_MD_CTX_Deleter> context(EVP_MD_CTX_create());

  TArray<uint8> pem_bytes;
  pem_bytes.AddUninitialized(public_key_pem.Len());
  ContainerUtils::TCHARToChar(public_key_pem, pem_bytes, pem_bytes.Num());

  TUniquePtr<BIO, BIO_Deleter> bio(BIO_new_mem_buf(pem_bytes.GetData(),
                                                   pem_bytes.Num()));

  if (bio.Get() == nullptr) {
    CFCORE_LOG(Error, TEXT("Failed allocating bio for PEM"));
    return false;
  }

  EVP_PKEY* PKey = PEM_read_bio_PUBKEY(bio.Get(), NULL, NULL, NULL);
  if (!PKey) {
    CFCORE_LOG(Error, TEXT("Failed parsing public key PEM"));
    return false;
  }

  EVP_PKEY_CTX* KeyContext = nullptr;
  EVP_DigestVerifyInit(context.Get(), &KeyContext, EVP_sha256(), nullptr, PKey);
  EVP_PKEY_CTX_set_rsa_padding(KeyContext, RSA_PKCS1_PSS_PADDING);

  // NOTE(twolf): Casting the, possibly, widchar_t string into a uint8_t array
  TArray<uint8> payload_bytes;
  payload_bytes.AddUninitialized(payload.Len());
  ContainerUtils::TCHARToChar(payload, payload_bytes, payload_bytes.Num());

  EVP_DigestVerifyUpdate(context.Get(),
                         payload_bytes.GetData(),
                         payload_bytes.Num());

  bool result = EVP_DigestVerifyFinal(
    context.Get(),
    const_cast<uint8*>(signature_buff.GetData()),
    signature_buff.Num()) == 1;

  return result;
#else
  CFCORE_LOG(Log, TEXT("OpenSSL not defined - skipping verification"));

  return true;
#endif
}

// -----------------------------------------------------------------------------
bool CryptoServiceImpl::ExtractJwtPayload(
  const FString& jwt,
  OUT FJwtPayload& out_payload) {

  TArray<FString> jwt_parts;
  int32 parts_count = jwt.ParseIntoArray(jwt_parts, TEXT("."), true);
  if (parts_count != 3) {
    CFCORE_LOG(Error, TEXT("jwt doesn't contain 3 parts"));
    return false;
  }

  FString json;
  const int kPayloadPartIndex = 1;
  bool success = FBase64::Decode(jwt_parts[kPayloadPartIndex], json);
  if (!success) {
    CFCORE_LOG(Error, TEXT("jwt doesn't contain 3 parts"));
    return false;
  }

  TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(json);

  TSharedPtr<FJsonObject> json_object;
  if (!FJsonSerializer::Deserialize(reader, json_object)) {
    CFCORE_LOG(Error, TEXT("failed to deserialize jwt payload"));
    return false;
  }

  if (!FJsonObjectConverter::JsonObjectToUStruct(json_object.ToSharedRef(),
                                                 &out_payload,
                                                 0,
                                                 0)) {
    CFCORE_LOG(Error, TEXT("failed to convert jwt payload to struct"));
    return false;
  }

  return true;
}
