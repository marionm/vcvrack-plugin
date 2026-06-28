#include <rack.hpp>

#include <string>

struct StartParamQuantity : rack::ParamQuantity {
  int index, totalLength;

  float getDisplayValue() override;
  void setDisplayValueString(std::string string) override;
};
