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
#include <services/buffer/buffer.h>
#include <Http.h>

namespace cfcore {

// -----------------------------------------------------------------------------
// This allows us keep an HttpResponse object (with it's internal buffer) alive
// while passing it between threads. Otherwise, we might have to start copying
// the response buffer to a newly allocated buffer - which can be very
// inefficient with large buffers
class HttpResponseBuffer : public IBuffer {
public:
  HttpResponseBuffer(FHttpResponsePtr response);
  virtual ~HttpResponseBuffer();

public:
  virtual const uint8* GetData() const override;
  virtual const int64 GetLength() const override;

private:
  FHttpResponsePtr http_response_;
};

} // namespace cfcore