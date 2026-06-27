#include <rack.hpp>

#include <string>

struct FilterParamQuantity : rack::ParamQuantity {
  std::string getString() override;
  std::string getDisplayValueString() override;
  void setDisplayValueString(std::string string) override;
};

