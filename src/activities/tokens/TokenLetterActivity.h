#pragma once

#include <string>
#include <vector>

#include "activities/UiListActivity.h"

// First level of the MTG token browser: an A-Z (+ '#') letter picker built
// from the files present in /tokens, with per-letter counts.
class TokenLetterActivity final : public UiListActivity {
  struct LetterBucket {
    char letter;
    int count;
  };
  std::vector<LetterBucket> buckets;
  std::vector<std::string> rowLabels;
  std::vector<freeink::ui::ListItem> rowItems;

  void loadBuckets();

  int listCount() const override { return static_cast<int>(buckets.size()); }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  const char* headerTitle() const override { return "Tokens"; }

 public:
  explicit TokenLetterActivity(GfxRenderer& renderer, MappedInputManager& mappedInput);
  void onEnter() override;
  void onExit() override;
};
