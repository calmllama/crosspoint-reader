#include "TokenLetterActivity.h"

#include <GfxRenderer.h>

#include <memory>

#include "MappedInputManager.h"
#include "TokenListActivity.h"
#include "TokenScan.h"
#include "components/UITheme.h"

namespace fui = freeink::ui;

TokenLetterActivity::TokenLetterActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
    : UiListActivity("TokenLetters", renderer, mappedInput) {}

void TokenLetterActivity::loadBuckets() {
  buckets.clear();
  rowLabels.clear();
  rowItems.clear();

  const auto files = TokenScan::listTokenFiles();
  for (const auto& f : files) {
    const char letter = TokenScan::letterFor(f);
    if (!buckets.empty() && buckets.back().letter == letter) {
      ++buckets.back().count;
    } else {
      buckets.push_back({letter, 1});
    }
  }

  rowLabels.reserve(buckets.size());
  rowItems.reserve(buckets.size());
  for (const auto& bucket : buckets) {
    std::string label(1, bucket.letter);
    label += "  (";
    label += std::to_string(bucket.count);
    label += ")";
    rowLabels.push_back(std::move(label));
    fui::ListItem item;
    item.label = rowLabels.back().c_str();
    item.actionValue = static_cast<int16_t>(rowItems.size());
    rowItems.push_back(item);
  }
}

void TokenLetterActivity::onEnter() {
  UiListActivity::onEnter();
  loadBuckets();
  if (nav.selected >= listCount()) nav.selected = listCount() > 0 ? listCount() - 1 : 0;
}

void TokenLetterActivity::onExit() {
  Activity::onExit();
  rowItems.clear();
  rowLabels.clear();
  buckets.clear();
}

void TokenLetterActivity::activateIndex(const int index) {
  if (index < 0 || index >= listCount()) return;
  app.clearTapFlash();
  startActivityForResult(std::make_unique<TokenListActivity>(renderer, mappedInput, buckets[index].letter),
                         [](const ActivityResult&) {});
}

void TokenLetterActivity::buildScreen(UiScreen& screen) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  screen.setContentMargin(fui::Insets{static_cast<int16_t>(metrics.topPadding + metrics.headerHeight), 0,
                                      static_cast<int16_t>(metrics.buttonHintsHeight), 0});
  screen.spacer(static_cast<int16_t>(metrics.verticalSpacing));

  if (buckets.empty()) {
    screen.centeredText("No token images found in /tokens");
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
