#pragma once

class BootKeyboardAPI {
 public:
  virtual size_t press(uint8_t) = 0;
  virtual void begin(void) = 0;
  virtual void end(void) = 0;
  virtual size_t release(uint8_t) = 0;
  virtual void releaseAll(void) = 0;
  virtual int sendReport(void) = 0;
  virtual uint8_t getProtocol(void) = 0;
  virtual void setProtocol(uint8_t protocol) = 0;
  virtual boolean isModifierActive(uint8_t k) = 0;
  virtual boolean wasModifierActive(uint8_t k) = 0;
};
