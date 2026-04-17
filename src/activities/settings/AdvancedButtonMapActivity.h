#pragma once

#include "activities/Activity.h"

class AdvancedButtonMapActivity final : public Activity {
 public:
  explicit AdvancedButtonMapActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("AdvancedButtonMap", renderer, mappedInput) {}

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;

 private:
  int selectedRow = 0;

  static const char* hardwareLabel(uint8_t hwIndex);
  static const char* actionLabel(uint8_t actionRaw);
  void bumpActionAtRow(int delta);
  [[nodiscard]] uint8_t hwIndexForRow(int row) const;
  [[nodiscard]] bool isLongRow(int row) const;
  [[nodiscard]] uint8_t* actionPtrForRow(int row);
};
