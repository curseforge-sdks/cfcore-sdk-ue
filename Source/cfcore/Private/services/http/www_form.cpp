/*MIT License

Copyright (c) 2022 Overwolf Ltd.

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
#include "www_form.h"
#include <Math/RandomStream.h>
#include <Serialization/MemoryArchive.h>
#include <GenericPlatform/GenericPlatformHttp.h>

namespace cfcore {

const char kBoundaryValidChars[] =
  "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const int kBoundaryValidCharsLength = sizeof(kBoundaryValidChars) - 1;

const uint64 kPngSignature = 0x0a1a0a0d474e5089;

const uint8 kDashBreaks[] = "--";
const uint8 kNewLine[] = "\r\n";
const uint8 kContentType[] = "Content-Type: ";
const uint8 kFieldNamePrefix[] = "Content-disposition: form-data; name=\"";
const uint8 kFilenamePrefix[] = "; filename=\"";

constexpr uint8 kMimeTypeTextPlain[] = "text/plain; charset=\"utf-8\"";
constexpr int64 kMimeTypeTextPlainLen = sizeof(kMimeTypeTextPlain) - 1;

constexpr uint8 kMimeTypeAppOctetStream[] = "application/octet-stream";
constexpr int64 kMimeTypeAppOctetStreamLen =
  sizeof(kMimeTypeAppOctetStream) - 1;

constexpr uint8 kMimeTypeAppJson[] = "application/json";
constexpr int64 kMimeTypeAppJsonLen = sizeof(kMimeTypeAppJson) - 1;

constexpr uint8 kMimeTypeImgPng[] = "image/png";
constexpr int64 kMimeTypeImgPngLen = sizeof(kMimeTypeImgPng) - 1;

const uint8 kQuote[] = "\"";

};

using namespace cfcore;

// -----------------------------------------------------------------------------
WWWForm::WWWForm() {
  Initialize();
}

// -----------------------------------------------------------------------------
WWWForm::~WWWForm() {
  Clear();
}

// -----------------------------------------------------------------------------
void WWWForm::Initialize() {
  Clear();
  GenerateRandomBoundary();
}

// -----------------------------------------------------------------------------
bool WWWForm::AddField(FString field_name, int64 value) {
  FString str_value = FString::Printf(TEXT("%lld"), value);
  return AddField(field_name, str_value);
}

// -----------------------------------------------------------------------------
bool WWWForm::AddField(FString field_name, int32 value) {
  FString str_value = FString::FromInt(value);
  return AddField(field_name, str_value);
}

// -----------------------------------------------------------------------------
bool WWWForm::AddField(FString field_name, bool value) {
  FString str_value = value ? "true" : "false";
  return AddField(field_name, str_value);
}

// -----------------------------------------------------------------------------
bool WWWForm::AddField(FString field_name, FString value) {
  if (field_name.IsEmpty()) {
    return false;
  }

  struct FormField form_field;
  form_field.name = field_name;
  form_field.value = value;
  form_field.mime_type = FieldType::TextPlain;

  form_fields_.Add(form_field);
  return true;
}

// -----------------------------------------------------------------------------
bool WWWForm::AddBinaryData(FString field_name,
                            TSharedPtr<TArray<uint8>> contents,
                            FString filename /*= FString("")*/,
                            FieldType mime_type/*= FieldType::AutoDetect*/) {
  if (field_name.IsEmpty()) {
    return false;
  }

  struct BinaryDataFormField form_field;
  form_field.name = field_name;
  form_field.contents = contents;
  form_field.filename = filename;
  form_field.mime_type = mime_type;

  SetImplicitBinaryDataFields(form_field);
  form_binary_data_field_.Add(form_field);
  return true;
}

// -----------------------------------------------------------------------------
FString WWWForm::GetContentTypeHeader() const {
  FString header = "application/x-www-form-urlencoded";

  if (form_binary_data_field_.Num() > 0) {
    FString boundary;
    boundary.AppendChars((const ANSICHAR*)boundary_.GetData(), boundary_.Num());
    header = FString::Format(
      TEXT("multipart/form-data; boundary=\"{0}\""),
      {*boundary});
  }

  return header;
}

// -----------------------------------------------------------------------------
TSharedPtr<TArray<uint8>> WWWForm::GetContents() const {
  auto output = MakeShared<TArray<uint8>>();
  FMemoryWriter mem_writer(*output);

  if (form_binary_data_field_.Num() == 0) {
    FillSimplePostBody(mem_writer);
    return output;
  }

  for (const struct FormField& field : form_fields_) {
    FillFormField(mem_writer, field);
  }

  for (const struct BinaryDataFormField& field : form_binary_data_field_) {
    FillBinaryDataFormField(mem_writer, field);
  }

  FillFormDataEnding(mem_writer);

  return output;
}

// -----------------------------------------------------------------------------
void WWWForm::GenerateRandomBoundary() {
  FRandomStream rand(FMath::Rand());
  rand.GenerateNewSeed();

  boundary_.AddUninitialized(40);

  for (int i = 0; i < 40; ++i) {
    int32 rand_val = rand.RandRange(0, kBoundaryValidCharsLength - 1);
    boundary_[i] = kBoundaryValidChars[rand_val];
  }
}

// -----------------------------------------------------------------------------
void WWWForm::Clear() {
  boundary_.Empty();
  form_fields_.Empty();
  form_binary_data_field_.Empty();
}

// -----------------------------------------------------------------------------
void WWWForm::SetImplicitBinaryDataFields(
  struct BinaryDataFormField& form_field) {
  bool is_png = IsPng(form_field);

  if (form_field.filename.IsEmpty()) {
    form_field.filename = form_field.name;
    if (is_png) {
      form_field.filename += FString(".png");
    } else {
      form_field.filename += FString(".dat");
    }
  }

  if (form_field.mime_type == FieldType::AutoDetect) {
    form_field.mime_type = FieldType::ApplicationOctetStream;
    if (is_png) {
      form_field.mime_type = FieldType::ImagePng;
    }
  }
}

// -----------------------------------------------------------------------------
bool WWWForm::IsPng(const struct BinaryDataFormField& form_field) {
  if (form_field.contents->Num() < sizeof(kPngSignature)) {
    return false;
  }

  uint64 content_sig = *((uint64*)form_field.contents->GetData());
  if (kPngSignature == content_sig) {
    return true;
  }

  return false;
}

// -----------------------------------------------------------------------------
void WWWForm::FillSimplePostBody(FMemoryWriter& mem_writer) const {
  int index = 0;

  for (const struct FormField& field : form_fields_) {
    if (index != 0) {
      mem_writer.Serialize((void*)"&", 1);
    }

    FString encoded_name = FGenericPlatformHttp::UrlEncode(field.name);
    FString encoded_value = FGenericPlatformHttp::UrlEncode(field.value);

    FTCHARToUTF8 utf8_encoded_name(*encoded_name);
    FTCHARToUTF8 utf8_encoded_value(*encoded_value);

    mem_writer.Serialize((void*)utf8_encoded_name.Get(),
                         utf8_encoded_name.Length());
    mem_writer.Serialize((void*)"=", 1);
    mem_writer.Serialize((void*)utf8_encoded_value.Get(),
                         utf8_encoded_value.Length());
    index++;
  }
}

// -----------------------------------------------------------------------------
void WWWForm::FillFormField(FMemoryWriter& mem_writer,
                            const struct FormField& form_field) const {
  FillFormFieldHeader(mem_writer, form_field.name, form_field.mime_type);

  FTCHARToUTF8 utf8_val(*form_field.value);
  mem_writer.Serialize((void*)utf8_val.Get(), utf8_val.Length());
}

// -----------------------------------------------------------------------------
void WWWForm::FillBinaryDataFormField(
  FMemoryWriter& mem_writer,
  const struct BinaryDataFormField& form_field) const {
  FillFormFieldHeader(mem_writer,
                      form_field.name,
                      form_field.mime_type,
                      form_field.filename);

  mem_writer.Serialize((void*)form_field.contents->GetData(),
                       form_field.contents->Num());
}

// -----------------------------------------------------------------------------
void WWWForm::FillFormFieldHeader(FMemoryWriter& mem_writer,
                                  FString field_name,
                                  FieldType mime_type,
                                  FString filename/*= FString("")*/) const {
  mem_writer.Serialize((void*)kNewLine, sizeof(kNewLine) - 1);
  mem_writer.Serialize((void*)kDashBreaks, sizeof(kDashBreaks) - 1);
  mem_writer.Serialize((void*)boundary_.GetData(), boundary_.Num());

  mem_writer.Serialize((void*)kNewLine, sizeof(kNewLine) - 1);
  mem_writer.Serialize((void*)kContentType, sizeof(kContentType) - 1);

  FillFormFieldMimeType(mem_writer, mime_type);

  mem_writer.Serialize((void*)kNewLine, sizeof(kNewLine) - 1);

  mem_writer.Serialize((void*)kFieldNamePrefix, sizeof(kFieldNamePrefix) - 1);

  // TODO(twolf): Consider checking and performing QPEncode
  FTCHARToUTF8 utf8_name(*field_name);
  mem_writer.Serialize((void*)utf8_name.Get(), utf8_name.Length());
  mem_writer.Serialize((void*)kQuote, sizeof(kQuote) - 1);

  if (!filename.IsEmpty()) {
    FillFilename(mem_writer, filename);
  }

  mem_writer.Serialize((void*)kNewLine, sizeof(kNewLine) - 1);
  mem_writer.Serialize((void*)kNewLine, sizeof(kNewLine) - 1);
}

// -----------------------------------------------------------------------------
void WWWForm::FillFormFieldMimeType(FMemoryWriter& mem_writer,
                                    FieldType type) const {
  switch (type) {
    case FieldType::ApplicationOctetStream:
      mem_writer.Serialize((void*)kMimeTypeAppOctetStream,
                           kMimeTypeAppOctetStreamLen);
      break;
    case FieldType::ImagePng:
      mem_writer.Serialize((void*)kMimeTypeImgPng, kMimeTypeImgPngLen);
      break;
    case ApplicationJson:
      mem_writer.Serialize((void*)kMimeTypeAppJson, kMimeTypeAppJsonLen);
      break;
    case FieldType::TextPlain:
    default:
      mem_writer.Serialize((void*)kMimeTypeTextPlain, kMimeTypeTextPlainLen);
  }
}

// -----------------------------------------------------------------------------
void WWWForm::FillFilename(FMemoryWriter& mem_writer, FString filename) const {
  mem_writer.Serialize((void*)kFilenamePrefix, sizeof(kFilenamePrefix) - 1);
  FTCHARToUTF8 utf8_filename(*filename);
  mem_writer.Serialize((void*)utf8_filename.Get(), utf8_filename.Length());
  mem_writer.Serialize((void*)kQuote, sizeof(kQuote) - 1);
}

// -----------------------------------------------------------------------------
void WWWForm::FillFormDataEnding(FMemoryWriter& mem_writer) const {
  mem_writer.Serialize((void*)kNewLine, sizeof(kNewLine) - 1);
  mem_writer.Serialize((void*)kDashBreaks, sizeof(kDashBreaks) - 1);
  mem_writer.Serialize((void*)boundary_.GetData(), boundary_.Num());
  mem_writer.Serialize((void*)kDashBreaks, sizeof(kDashBreaks) - 1);

  mem_writer.Serialize((void*)kNewLine, sizeof(kNewLine) - 1);
}