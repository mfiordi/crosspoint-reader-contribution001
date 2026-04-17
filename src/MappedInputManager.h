#pragma once

#include <HalGPIO.h>

#include "InputAction.h"

class MappedInputManager {
 public:
  enum class Button { Back, Confirm, Left, Right, Up, Down, Power, PageBack, PageForward };

  struct Labels {
    const char* btn1;
    const char* btn2;
    const char* btn3;
    const char* btn4;
  };

  explicit MappedInputManager(HalGPIO& gpio) : gpio(gpio) {}

  void update() const { gpio.update(); }

  // Call once per main loop after HalGPIO::update() when using advanced remapping.
  void afterGpioUpdate();

  void setBypassAdvancedRemap(bool bypass) { bypassAdvancedRemap = bypass; }
  [[nodiscard]] bool isAdvancedRemapBypassed() const { return bypassAdvancedRemap; }

  bool wasPressed(Button button) const;
  bool wasReleased(Button button) const;
  bool isPressed(Button button) const;
  bool wasAnyPressed() const;
  bool wasAnyReleased() const;
  unsigned long getHeldTime() const;
  Labels mapLabels(const char* back, const char* confirm, const char* previous, const char* next) const;
  // Returns the raw front button index that was pressed this frame (or -1 if none).
  int getPressedFrontButton() const;

  // Reader-only: consume a single queued action edge (clears one matching instance).
  bool consumeAction(InputAction action);

 private:
  HalGPIO& gpio;
  bool bypassAdvancedRemap = false;

  uint32_t hwPressStartMs[7] = {};

  // Reader activity input bits for the current frame (set in afterGpioUpdate, consumed from EpubReaderActivity).
  mutable uint32_t readerLatch = 0;
  mutable uint32_t extraLatch = 0;

  bool mapButton(Button button, bool (HalGPIO::*fn)(uint8_t) const) const;
  bool mapButtonLegacy(Button button, bool (HalGPIO::*fn)(uint8_t) const) const;

  void dispatchInputAction(InputAction action);
};
