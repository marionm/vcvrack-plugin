#pragma once

#include <rack.hpp>

struct GitHubTokenField : rack::ui::TextField {
  void draw(const DrawArgs& args) override;
};
