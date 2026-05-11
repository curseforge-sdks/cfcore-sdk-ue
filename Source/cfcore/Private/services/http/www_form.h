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
#pragma once
#include <CoreMinimal.h>
#include <JsonUtilities.h>

namespace cfcore {

// -----------------------------------------------------------------------------
// Class used to generate form data to send over POST/PUT requests
//
// NOTE(twolf): Currently, this was implemented in a fast and inefficient
// way - it keeps the binary data in memory and then, when requesting for the
// final content, it will create a new buffer that contains all the internal
// buffers combined.
//
// A more efficient way of implementing this would be to inhert from FArchive
// and implement a virtual Tell, Seek and Serialize functions that utilize the
// existing in-memory buffers.
//
// If we reach a point where this trade off makes sense - we could implement it
// as I mention above - however, it creates some coupling with the HTTP
// libraries used (e.g. IHttpRequest)
class WWWForm {
public:
  // ---------------------------------------------------------------------------
  enum FieldType {
    AutoDetect,
    TextPlain,
    ImagePng,
    ApplicationOctetStream,
    ApplicationJson
  };

  // ---------------------------------------------------------------------------
  struct FormField {
    FString name;
    FString value;
    FieldType mime_type;
  };

  // ---------------------------------------------------------------------------
  struct BinaryDataFormField {
    FString name;
    TSharedPtr<TArray<uint8>> contents;
    FString filename;
    FieldType mime_type;
  };

public:
  WWWForm();
  virtual ~WWWForm();

public:
  void Initialize();

  bool AddField(FString field_name, int64 value);
  bool AddField(FString field_name, int32 value);
  bool AddField(FString field_name, bool value);
  bool AddField(FString field_name, FString value);

  /**
   * Add an application/json field (based on a USTRUCT
   */
  template<typename InStructType>
  bool AddField(FString field_name, const InStructType& InStruct) {
    if (field_name.IsEmpty()) {
      return false;
    }

    FString json_data;
    if (!FJsonObjectConverter::UStructToJsonObjectString(InStruct, json_data)) {
      return false;
    }

    struct FormField form_field;
    form_field.name = field_name;
    form_field.value = json_data;
    form_field.mime_type = FieldType::ApplicationJson;

    form_fields_.Add(form_field);
    return true;
  }

  bool AddBinaryData(FString field_name,
                     TSharedPtr<TArray<uint8>> contents,
                     FString filename = FString(""),
                     FieldType mime_type = FieldType::AutoDetect);

  FString GetContentTypeHeader() const;
  TSharedPtr<TArray<uint8>> GetContents() const;

private:
  void Clear();
  void GenerateRandomBoundary();
  void SetImplicitBinaryDataFields(struct BinaryDataFormField& form_field);
  bool IsPng(const struct BinaryDataFormField& form_field);

  void FillSimplePostBody(FMemoryWriter& mem_writer) const;
  void FillFormField(FMemoryWriter& mem_writer,
                     const struct FormField& form_field) const;
  void FillBinaryDataFormField(
    FMemoryWriter& mem_writer,
    const struct BinaryDataFormField& form_field) const;
  void FillFormFieldHeader(FMemoryWriter& mem_writer,
                           FString field_name,
                           FieldType mime_type,
                           FString filename = FString("")) const;
  void FillFormFieldMimeType(FMemoryWriter& mem_writer, FieldType type) const;
  void FillFilename(FMemoryWriter& mem_writer, FString filename) const;
  void FillFormDataEnding(FMemoryWriter& mem_writer) const;

private:
  TArray<uint8> boundary_;
  TArray<struct FormField> form_fields_;
  TArray<struct BinaryDataFormField> form_binary_data_field_;
};

} // namespace cfcore
