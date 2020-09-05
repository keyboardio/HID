/*
Copyright (c) 2015-2019 Keyboard.io, Inc

See the readme for credit to other people.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#pragma once

#ifdef KALEIDOSCOPE_VIRTUAL_BUILD
#undef T
#undef U
#undef min
#undef max
#undef TEST
#include <functional>
#endif

#include <stdint.h>

class HIDReportObserver {
 public:

#ifdef KALEIDOSCOPE_VIRTUAL_BUILD
  typedef std::function<void(uint8_t, const void*, int, int)> SendReportHook;
#else  // KALEIDOSCOPE_VIRTUAL_BUILD
  typedef void(*SendReportHook)(
      uint8_t id, const void* data, int len, int result);
#endif  // KALEIDOSCOPE_VIRTUAL_BUILD

  static void observeReport(uint8_t id, const void* data, int len, int result) {
    if (send_report_hook_) {
#ifdef KALEIDOSCOPE_VIRTUAL_BUILD
      send_report_hook_(id, data, len, result);
#else  // KALEIDOSCOPE_VIRTUAL_BUILD
      (*send_report_hook_)(id, data, len, result);
#endif  // KALEIDOSCOPE_VIRTUAL_BUILD
    }
  }
      
    static SendReportHook currentHook() { return send_report_hook_; }
    
    static SendReportHook resetHook(SendReportHook new_hook) {
       auto previous_hook = send_report_hook_;
       send_report_hook_ = new_hook; 
       return previous_hook;
   }
      
   private:
      
      static SendReportHook send_report_hook_;
};
