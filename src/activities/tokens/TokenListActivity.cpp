#include "TokenListActivity.h"

#include <GfxRenderer.h>

#include <cstdio>
#include <memory>

#include "MappedInputManager.h"
#include "TokenScan.h"
#include "activities/util/BmpViewerActivity.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

TokenListActivity::TokenListActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, const char letter)
    : UiListActivity("TokenList", renderer, mappedInput), letter(letter) {
  snprintf(headerText, sizeof(headerText), "Tokens: %c", letter);
}

void TokenListActivity::loadFiles() {
  files.clear();
  rowLabels.clear();
  rowItems.clear();

  for (auto& f : TokenScan::listTokenFiles()) {
    if (TokenScan::letterFor(f) == letter) {
      files.push_back(std::move(f));
    }
  }

  rowLabels.reserve(files.size());
  rowItems.reserve(files.size());
  for (const auto& f : files) {
    rowLabels.push_back(TokenScan::displayName(f));
    fui::ListItem item;
    item.label = rowLabels.back().c_str();
    item.actionValue = static_cast<int16_t>(rowItems.size());
    rowItems.push_back(item);
  }
}

void TokenListActivity::onEnter() {
  UiListActivity::onEnter();
  loadFiles();
  if (nav.selected >= listCount()) nav.selected = listCount() > 0 ? listCount() - 1 : 0;
}

void TokenListActivity::onExit() {
  Activity::onExit();
  rowItems.clear();
  rowLabels.clear();
  files.clear();
}

void TokenListActivity::activateIndex(const int index) {
  if (index < 0 || index >= listCount()) return;
  app.clearTapFlash();
  std::string path(TokenScan::TOKENS_DIR);
  path += "/";
  path += files[index];
  startActivityForResult(std::make_unique<BmpViewerActivity>(renderer, mappedInput, std::move(path)),
                         [](const ActivityResult&) {});
}

void TokenListActivity::buildScreen(UiScreen& screen) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  screen.setContentMargin(fui::Insets{static_cast<int16_t>(metrics.topPadding + metrics.headerHeight), 0,
                                      static_cast<int16_t>(metrics.buttonHintsHeight), 0});
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));

  if (files.empty()) {
    screen.centeredText("No tokens for this letter");
    return;
  }

  fui::ListProps props;
  props.items = rowItems.data();
  props.count = static_cast<uint16_t>(rowItems.size());
  props.action = ACTION_ROW;
  props.inputMask = fui::InputTouch;
  syncListViewport(screen, props);
  screen.list(props);
}
