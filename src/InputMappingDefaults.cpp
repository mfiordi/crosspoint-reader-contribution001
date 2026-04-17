#include "InputMappingDefaults.h"

#include "InputAction.h"

#include <algorithm>

namespace {

struct SideHw {
  uint8_t pageBack;
  uint8_t pageForward;
};

constexpr SideHw kSideLayouts[] = {
    {4, 5},  // PREV_NEXT: UP, DOWN
    {5, 4},  // NEXT_PREV
};

}  // namespace

namespace InputMappingDefaults {

void applyStockInputMapping(CrossPointSettings& s) {
  for (uint8_t i = 0; i < 7; i++) {
    s.inputMapShort[i] = static_cast<uint8_t>(InputAction::None);
    s.inputMapLong[i] = static_cast<uint8_t>(InputAction::None);
  }

  const SideHw side = kSideLayouts[std::min<uint8_t>(s.sideButtonLayout, 1)];

  const uint8_t backHw = s.frontButtonBack;
  const uint8_t confirmHw = s.frontButtonConfirm;
  const uint8_t leftHw = s.frontButtonLeft;
  const uint8_t rightHw = s.frontButtonRight;

  if (backHw < 7) {
    s.inputMapShort[backHw] = static_cast<uint8_t>(InputAction::ReaderBackShort);
    s.inputMapLong[backHw] = static_cast<uint8_t>(InputAction::ReaderBackLong);
  }
  if (confirmHw < 7) {
    s.inputMapShort[confirmHw] = static_cast<uint8_t>(InputAction::ReaderOpenMenu);
    s.inputMapLong[confirmHw] = static_cast<uint8_t>(InputAction::None);
  }

  const InputAction chapterPrev =
      s.longPressChapterSkip ? InputAction::ReaderChapterPrev : InputAction::None;
  const InputAction chapterNext =
      s.longPressChapterSkip ? InputAction::ReaderChapterNext : InputAction::None;

  auto bindPagePair = [&](uint8_t hw, InputAction pageShort, InputAction chapterLong) {
    if (hw >= 7) {
      return;
    }
    s.inputMapShort[hw] = static_cast<uint8_t>(pageShort);
    s.inputMapLong[hw] = static_cast<uint8_t>(chapterLong);
  };

  bindPagePair(leftHw, InputAction::ReaderPagePrev, chapterPrev);
  bindPagePair(rightHw, InputAction::ReaderPageNext, chapterNext);
  bindPagePair(side.pageBack, InputAction::ReaderPagePrev, chapterPrev);
  bindPagePair(side.pageForward, InputAction::ReaderPageNext, chapterNext);

  if (4 < 7) {
    s.inputMapShort[4] = static_cast<uint8_t>(InputAction::UiUp);
    s.inputMapLong[4] = static_cast<uint8_t>(InputAction::None);
  }
  if (5 < 7) {
    s.inputMapShort[5] = static_cast<uint8_t>(InputAction::UiDown);
    s.inputMapLong[5] = static_cast<uint8_t>(InputAction::None);
  }

  // Power: short click only (long press is sleep / power-off path in main).
  uint8_t pwrShort = static_cast<uint8_t>(InputAction::None);
  if (s.shortPwrBtn == CrossPointSettings::SHORT_PWRBTN::PAGE_TURN) {
    pwrShort = static_cast<uint8_t>(InputAction::ReaderPageNext);
  } else if (s.shortPwrBtn == CrossPointSettings::SHORT_PWRBTN::FORCE_REFRESH) {
    pwrShort = static_cast<uint8_t>(InputAction::ScreenForceRefresh);
  }
  s.inputMapShort[6] = pwrShort;
  s.inputMapLong[6] = static_cast<uint8_t>(InputAction::None);
}

unsigned long longThresholdMsForHw(const CrossPointSettings& s, uint8_t hwIndex) {
  if (hwIndex == s.frontButtonBack) {
    return 1000;  // Keep in sync with ReaderUtils::GO_HOME_MS
  }
  const SideHw side = kSideLayouts[std::min<uint8_t>(s.sideButtonLayout, 1)];
  if (s.longPressChapterSkip &&
      (hwIndex == side.pageBack || hwIndex == side.pageForward || hwIndex == s.frontButtonLeft ||
       hwIndex == s.frontButtonRight)) {
    return 700;
  }
  const unsigned long d = s.inputLongPressDefaultMs == 0 ? 700 : s.inputLongPressDefaultMs;
  return d;
}

}  // namespace InputMappingDefaults
