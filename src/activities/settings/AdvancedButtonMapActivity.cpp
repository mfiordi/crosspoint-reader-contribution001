#include "AdvancedButtonMapActivity.h"

#include <cstdio>

#include <GfxRenderer.h>
#include <I18n.h>

#include "CrossPointSettings.h"
#include "InputAction.h"
#include "InputMappingDefaults.h"
#include "components/UITheme.h"
#include "fontIds.h"

namespace {
constexpr int kRowCount = 14;
}

void AdvancedButtonMapActivity::onEnter() {
  Activity::onEnter();
  mappedInput.setBypassAdvancedRemap(true);
  selectedRow = 0;

  bool allNone = true;
  for (uint8_t i = 0; i < 7; i++) {
    if (SETTINGS.inputMapShort[i] != 0 || SETTINGS.inputMapLong[i] != 0) {
      allNone = false;
      break;
    }
  }
  if (allNone) {
    InputMappingDefaults::applyStockInputMapping(SETTINGS);
  }

  requestUpdate();
}

void AdvancedButtonMapActivity::onExit() {
  mappedInput.setBypassAdvancedRemap(false);
  Activity::onExit();
}

uint8_t AdvancedButtonMapActivity::hwIndexForRow(const int row) const {
  if (row < 0 || row >= kRowCount) {
    return 0;
  }
  return static_cast<uint8_t>(row % 7);
}

bool AdvancedButtonMapActivity::isLongRow(const int row) const { return row >= 7; }

uint8_t* AdvancedButtonMapActivity::actionPtrForRow(const int row) {
  const uint8_t hw = hwIndexForRow(row);
  return isLongRow(row) ? &SETTINGS.inputMapLong[hw] : &SETTINGS.inputMapShort[hw];
}

void AdvancedButtonMapActivity::bumpActionAtRow(const int delta) {
  uint8_t* ptr = actionPtrForRow(selectedRow);
  if (!ptr) {
    return;
  }
  const int count = static_cast<int>(InputAction::COUNT);
  int v = static_cast<int>(*ptr);
  v = (v + delta) % count;
  if (v < 0) {
    v += count;
  }
  *ptr = static_cast<uint8_t>(v);
  CrossPointSettings::validateInputActionMaps(SETTINGS);
  requestUpdate();
}

void AdvancedButtonMapActivity::loop() {
  if (mappedInput.wasPressed(MappedInputManager::Button::Up)) {
    selectedRow = (selectedRow + kRowCount - 1) % kRowCount;
    requestUpdate();
    return;
  }
  if (mappedInput.wasPressed(MappedInputManager::Button::Down)) {
    selectedRow = (selectedRow + 1) % kRowCount;
    requestUpdate();
    return;
  }

  if (mappedInput.wasPressed(MappedInputManager::Button::Confirm)) {
    bumpActionAtRow(1);
    return;
  }

  if (mappedInput.wasPressed(MappedInputManager::Button::Left)) {
    bumpActionAtRow(-1);
    return;
  }
  if (mappedInput.wasPressed(MappedInputManager::Button::Right)) {
    bumpActionAtRow(1);
    return;
  }

  if (mappedInput.wasPressed(MappedInputManager::Button::Back)) {
    SETTINGS.saveToFile();
    finish();
    return;
  }
}

const char* AdvancedButtonMapActivity::hardwareLabel(const uint8_t hwIndex) {
  switch (hwIndex) {
    case 0:
      return tr(STR_HW_BACK_LABEL);
    case 1:
      return tr(STR_HW_CONFIRM_LABEL);
    case 2:
      return tr(STR_HW_LEFT_LABEL);
    case 3:
      return tr(STR_HW_RIGHT_LABEL);
    case 4:
      return tr(STR_DIR_UP);
    case 5:
      return tr(STR_DIR_DOWN);
    case 6:
    default:
      return tr(STR_HW_POWER_LABEL);
  }
}

const char* AdvancedButtonMapActivity::actionLabel(const uint8_t actionRaw) {
  if (!isValidInputAction(actionRaw)) {
    return tr(STR_ACTION_NONE);
  }
  switch (static_cast<InputAction>(actionRaw)) {
    case InputAction::None:
      return tr(STR_ACTION_NONE);
    case InputAction::UiBack:
      return tr(STR_ACTION_UI_BACK);
    case InputAction::UiConfirm:
      return tr(STR_ACTION_UI_CONFIRM);
    case InputAction::UiUp:
      return tr(STR_ACTION_UI_UP);
    case InputAction::UiDown:
      return tr(STR_ACTION_UI_DOWN);
    case InputAction::UiLeft:
      return tr(STR_ACTION_UI_LEFT);
    case InputAction::UiRight:
      return tr(STR_ACTION_UI_RIGHT);
    case InputAction::ReaderOpenMenu:
      return tr(STR_ACTION_READER_MENU);
    case InputAction::ReaderBackShort:
      return tr(STR_ACTION_READER_BACK_SHORT);
    case InputAction::ReaderBackLong:
      return tr(STR_ACTION_READER_BACK_LONG);
    case InputAction::ReaderPagePrev:
      return tr(STR_ACTION_PAGE_PREV);
    case InputAction::ReaderPageNext:
      return tr(STR_ACTION_PAGE_NEXT);
    case InputAction::ReaderChapterPrev:
      return tr(STR_ACTION_CHAPTER_PREV);
    case InputAction::ReaderChapterNext:
      return tr(STR_ACTION_CHAPTER_NEXT);
    case InputAction::FontIncrease:
      return tr(STR_ACTION_FONT_INC);
    case InputAction::FontDecrease:
      return tr(STR_ACTION_FONT_DEC);
    case InputAction::CycleOrientation:
      return tr(STR_ACTION_CYCLE_ORIENTATION);
    case InputAction::Screenshot:
      return tr(STR_ACTION_SCREENSHOT);
    case InputAction::ToggleAlternateBook:
      return tr(STR_ACTION_TOGGLE_BOOK);
    case InputAction::ScreenForceRefresh:
      return tr(STR_ACTION_FORCE_REFRESH);
    default:
      return tr(STR_ACTION_NONE);
  }
}

void AdvancedButtonMapActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();

  char rowTitles[kRowCount][80];
  for (int i = 0; i < kRowCount; i++) {
    const uint8_t hw = hwIndexForRow(i);
    const char* kind = isLongRow(i) ? tr(STR_KIND_LONG) : tr(STR_KIND_SHORT);
    snprintf(rowTitles[i], sizeof(rowTitles[i]), "%s — %s", hardwareLabel(hw), kind);
  }

  renderer.clearScreen();

  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight},
                 tr(STR_ADVANCED_INPUT_MAP_TITLE));
  GUI.drawSubHeader(renderer, Rect{0, metrics.topPadding + metrics.headerHeight, pageWidth, metrics.tabBarHeight},
                    tr(STR_ADVANCED_BUTTON_MAP));

  const int topOffset = metrics.topPadding + metrics.headerHeight + metrics.tabBarHeight + metrics.verticalSpacing;
  const int contentHeight = pageHeight - topOffset - metrics.buttonHintsHeight - metrics.verticalSpacing;

  GUI.drawList(
      renderer, Rect{0, topOffset, pageWidth, contentHeight}, kRowCount, selectedRow,
      [&](int index) { return rowTitles[index]; }, nullptr, nullptr,
      [&](int index) { return actionLabel(*actionPtrForRow(index)); }, true);

  GUI.drawHelpText(renderer,
                   Rect{0, pageHeight - metrics.buttonHintsHeight - metrics.contentSidePadding, pageWidth,
                        metrics.buttonHintsHeight},
                   tr(STR_ADVANCED_MAP_HINTS));

  GUI.drawButtonHints(renderer, tr(STR_BACK), tr(STR_CONFIRM), tr(STR_DIR_LEFT), tr(STR_DIR_RIGHT));
  renderer.displayBuffer();
}
