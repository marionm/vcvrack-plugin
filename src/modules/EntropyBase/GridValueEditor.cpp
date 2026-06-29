#include "GridValueEditor.hpp"
#include "../../helpers/clamp.hpp"

using namespace rack;

void GridValueEditor::spawn(int index, float* pValue, const math::Vec& pos) {
  ui::Menu* menu = new ui::Menu();
  menu->box.pos = pos;

  ui::MenuLabel* menuLabel = new ui::MenuLabel();
  menuLabel->text = string::f("Index %i value", index), 
  menu->addChild(menuLabel);

  ui::MenuEntry* entry = new ui::MenuEntry();
  entry->box.size = math::Vec(120.f, 25.f);
  menu->addChild(entry);

  GridValueEditor* inputField = new GridValueEditor(pValue);
  inputField->box.size = entry->box.size;
  entry->addChild(inputField);

  ui::MenuOverlay* overlay = new ui::MenuOverlay();
  overlay->addChild(menu);
  APP->scene->addChild(overlay);
}

GridValueEditor::GridValueEditor(float* pValue) : pValue(pValue) {
  if (pValue) {
    this->text = rack::string::f("%.2f", *pValue);
  }
}

void GridValueEditor::onAction(const ActionEvent& e) {
  ui::TextField::onAction(e);
  if (pValue) {
    try {
      *pValue = clamp01(std::stof(this->text));
    } catch (...) {}
  }

  ui::Menu* menu = getAncestorOfType<ui::Menu>();
  if (menu) {
    ui::MenuOverlay* overlay = menu->getAncestorOfType<ui::MenuOverlay>();
    if (overlay) {
      overlay->requestDelete();
    } else {
      menu->requestDelete();
    }
  }
}

void GridValueEditor::draw(const DrawArgs& args) {
  ui::TextField::draw(args);

  if (!wasFocused) {
    APP->event->setSelectedWidget(this);
    this->selectAll();
    wasFocused = true;
  }
}

