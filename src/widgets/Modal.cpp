#include "Modal.hpp"
#include "Popup.hpp"

using namespace rack;

// TODO: Autosizing based on children would be cool...
struct ModalButton : ui::Button {
  ModalButton(Modal* modal)
    : modal(modal)
  {
    box.size = Vec(64, 21);
  }

  Modal* modal;
};

struct ModalSaveButton : ModalButton {
  ModalSaveButton(Modal* modal)
    : ModalButton(modal)
  {
    text = "Save";
    box.pos = Vec(modal->box.size.x - box.size.x - 14.f, modal->box.size.y - box.size.y - 8.f);
  }

  void onAction(const event::Action& e) override {
    try {
      if (modal->onSave()) {
        Popup::close(modal);
      }
    } catch(...) {
      Popup::close(modal);
    }
  }
};

struct ModalCancelButton : ModalButton {
  ModalCancelButton(Modal* modal)
    : ModalButton(modal)
  {
    text = "Cancel";
    box.pos = Vec(modal->box.size.x - (box.size.x * 2) - 22.f, modal->box.size.y - box.size.y - 8.f);
  }

  void onAction(const event::Action& e) override {
    Popup::close(modal);
  }
};

Modal::Modal(int width, int height) {
  box.size = Vec(width, height);

  addChild(new ModalSaveButton(this));
  addChild(new ModalCancelButton(this));

  new Popup(this);
}

void Modal::onOpen() {
}

void Modal::close(Modal* modal) {
  Popup::close(modal);
}

// Was unable to just inherit from ui::Menu, unfortunately
void Modal::draw(const DrawArgs& args) {
  if (!opened) {
    opened = true;
    onOpen();
  }

  nvgBeginPath(args.vg);
  nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 6.f);
  nvgFillColor(args.vg, nvgRGB(40, 40, 40));
  nvgFill(args.vg);

  nvgStrokeWidth(args.vg, 1.f);
  nvgStrokeColor(args.vg, nvgRGB(80, 80, 80));
  nvgStroke(args.vg);

  Widget::draw(args);
}
