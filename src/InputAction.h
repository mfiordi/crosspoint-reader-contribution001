#pragma once

#include <cstdint>

// Remappable logical actions (stored as uint8_t in settings). Extend by inserting before COUNT and
// updating InputMappingDefaults + dispatch tables in MappedInputManager + action picker UI.
enum class InputAction : uint8_t {
  None = 0,
  UiBack,
  UiConfirm,
  UiUp,
  UiDown,
  UiLeft,
  UiRight,
  ReaderOpenMenu,
  ReaderBackShort,
  ReaderBackLong,
  ReaderPagePrev,
  ReaderPageNext,
  ReaderChapterPrev,
  ReaderChapterNext,
  FontIncrease,
  FontDecrease,
  CycleOrientation,
  Screenshot,
  ToggleAlternateBook,
  ScreenForceRefresh,
  COUNT
};

inline bool isValidInputAction(uint8_t v) { return v < static_cast<uint8_t>(InputAction::COUNT); }
