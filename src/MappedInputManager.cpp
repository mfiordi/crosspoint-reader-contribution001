#include "MappedInputManager.h"

#include "CrossPointSettings.h"
#include "InputMappingDefaults.h"
#include "activities/ActivityManager.h"

namespace {

using S = CrossPointSettings;

struct SideLayoutMap {
  uint8_t pageBack;
  uint8_t pageForward;
};

constexpr SideLayoutMap kSideLayouts[] = {
    {HalGPIO::BTN_UP, HalGPIO::BTN_DOWN},
    {HalGPIO::BTN_DOWN, HalGPIO::BTN_UP},
};

constexpr uint32_t kReaderOpenMenu = 1u << 0;
constexpr uint32_t kReaderBackShort = 1u << 1;
constexpr uint32_t kReaderBackLong = 1u << 2;
constexpr uint32_t kReaderPagePrev = 1u << 3;
constexpr uint32_t kReaderPageNext = 1u << 4;
constexpr uint32_t kReaderChapterPrev = 1u << 5;
constexpr uint32_t kReaderChapterNext = 1u << 6;

constexpr uint32_t kExtraFontInc = 1u << 0;
constexpr uint32_t kExtraFontDec = 1u << 1;
constexpr uint32_t kExtraOrient = 1u << 2;
constexpr uint32_t kExtraScreenshot = 1u << 3;
constexpr uint32_t kExtraToggleBook = 1u << 4;
constexpr uint32_t kExtraForceRefresh = 1u << 5;

bool isReaderOnlyAction(const InputAction a) {
  switch (a) {
    case InputAction::ReaderOpenMenu:
    case InputAction::ReaderBackShort:
    case InputAction::ReaderBackLong:
    case InputAction::ReaderPagePrev:
    case InputAction::ReaderPageNext:
    case InputAction::ReaderChapterPrev:
    case InputAction::ReaderChapterNext:
    case InputAction::FontIncrease:
    case InputAction::FontDecrease:
    case InputAction::CycleOrientation:
    case InputAction::Screenshot:
    case InputAction::ToggleAlternateBook:
      return true;
    default:
      return false;
  }
}

void applyActionToLatches(const InputAction a, uint32_t& reader, uint32_t& extra) {
  switch (a) {
    case InputAction::ReaderOpenMenu:
      reader |= kReaderOpenMenu;
      break;
    case InputAction::ReaderBackShort:
      reader |= kReaderBackShort;
      break;
    case InputAction::ReaderBackLong:
      reader |= kReaderBackLong;
      break;
    case InputAction::ReaderPagePrev:
      reader |= kReaderPagePrev;
      break;
    case InputAction::ReaderPageNext:
      reader |= kReaderPageNext;
      break;
    case InputAction::ReaderChapterPrev:
      reader |= kReaderChapterPrev;
      break;
    case InputAction::ReaderChapterNext:
      reader |= kReaderChapterNext;
      break;
    case InputAction::FontIncrease:
      extra |= kExtraFontInc;
      break;
    case InputAction::FontDecrease:
      extra |= kExtraFontDec;
      break;
    case InputAction::CycleOrientation:
      extra |= kExtraOrient;
      break;
    case InputAction::Screenshot:
      extra |= kExtraScreenshot;
      break;
    case InputAction::ToggleAlternateBook:
      extra |= kExtraToggleBook;
      break;
    case InputAction::ScreenForceRefresh:
      extra |= kExtraForceRefresh;
      break;
    default:
      break;
  }
}

uint32_t readerMaskForAction(const InputAction a) {
  switch (a) {
    case InputAction::ReaderOpenMenu:
      return kReaderOpenMenu;
    case InputAction::ReaderBackShort:
      return kReaderBackShort;
    case InputAction::ReaderBackLong:
      return kReaderBackLong;
    case InputAction::ReaderPagePrev:
      return kReaderPagePrev;
    case InputAction::ReaderPageNext:
      return kReaderPageNext;
    case InputAction::ReaderChapterPrev:
      return kReaderChapterPrev;
    case InputAction::ReaderChapterNext:
      return kReaderChapterNext;
    default:
      return 0;
  }
}

uint32_t extraMaskForAction(const InputAction a) {
  switch (a) {
    case InputAction::FontIncrease:
      return kExtraFontInc;
    case InputAction::FontDecrease:
      return kExtraFontDec;
    case InputAction::CycleOrientation:
      return kExtraOrient;
    case InputAction::Screenshot:
      return kExtraScreenshot;
    case InputAction::ToggleAlternateBook:
      return kExtraToggleBook;
    case InputAction::ScreenForceRefresh:
      return kExtraForceRefresh;
    default:
      return 0;
  }
}

}  // namespace

void MappedInputManager::afterGpioUpdate() {
  readerLatch = 0;
  extraLatch = 0;

  if (!SETTINGS.advancedButtonRemap || bypassAdvancedRemap) {
    return;
  }

  const bool inReader = crosspointIsActiveReaderActivity();

  for (uint8_t hw = 0; hw < 7; hw++) {
    if (gpio.wasPressed(hw)) {
      hwPressStartMs[hw] = millis();
    }
  }

  for (uint8_t hw = 0; hw < 7; hw++) {
    if (!gpio.wasReleased(hw)) {
      continue;
    }

    const unsigned long start = hwPressStartMs[hw];
    const unsigned long duration = (start == 0) ? 0 : (millis() - start);
    hwPressStartMs[hw] = 0;

    const bool isPower = (hw == HalGPIO::BTN_POWER);
    const unsigned long thr = isPower ? 0 : InputMappingDefaults::longThresholdMsForHw(SETTINGS, hw);
    const bool useLong = !isPower && duration >= thr;

    const uint8_t raw = useLong ? SETTINGS.inputMapLong[hw] : SETTINGS.inputMapShort[hw];
    if (!isValidInputAction(raw)) {
      continue;
    }
    const auto action = static_cast<InputAction>(raw);
    if (action == InputAction::None) {
      continue;
    }

    if (action == InputAction::ScreenForceRefresh) {
      applyActionToLatches(action, readerLatch, extraLatch);
      continue;
    }

    if (isReaderOnlyAction(action) && !inReader) {
      continue;
    }

    applyActionToLatches(action, readerLatch, extraLatch);
  }
}

bool MappedInputManager::consumeAction(const InputAction action) {
  const uint32_t rm = readerMaskForAction(action);
  if (rm != 0 && (readerLatch & rm) != 0) {
    readerLatch &= ~rm;
    return true;
  }
  const uint32_t em = extraMaskForAction(action);
  if (em != 0 && (extraLatch & em) != 0) {
    extraLatch &= ~em;
    return true;
  }
  return false;
}

bool MappedInputManager::mapButtonLegacy(const Button button, bool (HalGPIO::*fn)(uint8_t) const) const {
  const auto sideLayout = static_cast<S::SIDE_BUTTON_LAYOUT>(SETTINGS.sideButtonLayout);
  const auto& side = kSideLayouts[sideLayout];

  switch (button) {
    case Button::Back:
      return (gpio.*fn)(SETTINGS.frontButtonBack);
    case Button::Confirm:
      return (gpio.*fn)(SETTINGS.frontButtonConfirm);
    case Button::Left:
      return (gpio.*fn)(SETTINGS.frontButtonLeft);
    case Button::Right:
      return (gpio.*fn)(SETTINGS.frontButtonRight);
    case Button::Up:
      return (gpio.*fn)(HalGPIO::BTN_UP);
    case Button::Down:
      return (gpio.*fn)(HalGPIO::BTN_DOWN);
    case Button::Power:
      return (gpio.*fn)(HalGPIO::BTN_POWER);
    case Button::PageBack:
      return (gpio.*fn)(side.pageBack);
    case Button::PageForward:
      return (gpio.*fn)(side.pageForward);
  }
  return false;
}

bool MappedInputManager::mapButton(const Button button, bool (HalGPIO::*fn)(uint8_t) const) const {
  if (SETTINGS.advancedButtonRemap && !bypassAdvancedRemap && crosspointIsActiveReaderActivity()) {
    // Epub reader loop consumes advanced actions explicitly; suppress legacy logical mapping to avoid double
    // triggers while reading.
    (void)button;
    (void)fn;
    return false;
  }
  return mapButtonLegacy(button, fn);
}

bool MappedInputManager::wasPressed(const Button button) const { return mapButton(button, &HalGPIO::wasPressed); }

bool MappedInputManager::wasReleased(const Button button) const { return mapButton(button, &HalGPIO::wasReleased); }

bool MappedInputManager::isPressed(const Button button) const { return mapButton(button, &HalGPIO::isPressed); }

bool MappedInputManager::wasAnyPressed() const { return gpio.wasAnyPressed(); }

bool MappedInputManager::wasAnyReleased() const { return gpio.wasAnyReleased(); }

unsigned long MappedInputManager::getHeldTime() const { return gpio.getHeldTime(); }

MappedInputManager::Labels MappedInputManager::mapLabels(const char* back, const char* confirm, const char* previous,
                                                         const char* next) const {
  auto labelForHardware = [&](uint8_t hw) -> const char* {
    if (hw == SETTINGS.frontButtonBack) {
      return back;
    }
    if (hw == SETTINGS.frontButtonConfirm) {
      return confirm;
    }
    if (hw == SETTINGS.frontButtonLeft) {
      return previous;
    }
    if (hw == SETTINGS.frontButtonRight) {
      return next;
    }
    return "";
  };

  return {labelForHardware(HalGPIO::BTN_BACK), labelForHardware(HalGPIO::BTN_CONFIRM),
          labelForHardware(HalGPIO::BTN_LEFT), labelForHardware(HalGPIO::BTN_RIGHT)};
}

int MappedInputManager::getPressedFrontButton() const {
  if (gpio.wasPressed(HalGPIO::BTN_BACK)) {
    return HalGPIO::BTN_BACK;
  }
  if (gpio.wasPressed(HalGPIO::BTN_CONFIRM)) {
    return HalGPIO::BTN_CONFIRM;
  }
  if (gpio.wasPressed(HalGPIO::BTN_LEFT)) {
    return HalGPIO::BTN_LEFT;
  }
  if (gpio.wasPressed(HalGPIO::BTN_RIGHT)) {
    return HalGPIO::BTN_RIGHT;
  }
  return -1;
}
