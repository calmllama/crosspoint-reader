#pragma once

#include <string>
#include <vector>

#include "activities/UiListActivity.h"

// Second level of the MTG token browser: the tokens whose names start with a
// given letter. Selecting one opens the stock BmpViewerActivity full screen
// (with sibling paging and set-as-sleep-cover for free).
class TokenListActivity final : public UiListActivity {
  const char letter;
  char headerText[12] = {0};
  std::vector<std::string> files;
  std::vector<std::string> rowLabels;
  std::vector<freeink::ui::ListItem> rowItems;

  void loadFiles();

  int listCount() const override { return static_cast<int>(files.size()); }
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  const char* headerTitle() const override { return headerText; }

 public:
  TokenListActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, char letter);
  void onEnter() override;
  void onExit() override;
};
