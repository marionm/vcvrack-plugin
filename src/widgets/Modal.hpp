#pragma once

#include <rack.hpp>

// A popup that is auto-centered and has a save and cancel button
// Just instantiate to spawn
// TODO: Autosize based on added children?
struct Modal : rack::OpaqueWidget {
  Modal(int width, int height);

  virtual void onOpen();
  virtual bool onSave() = 0;
  static void close(Modal* modal);

private:
  void draw(const DrawArgs& args) override;

  bool opened = false;
};
