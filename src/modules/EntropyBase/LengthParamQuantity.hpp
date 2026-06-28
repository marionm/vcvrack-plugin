#include <rack.hpp>

#include <string>

struct LengthParamQuantity : rack::ParamQuantity {
  int length, totalLength;

  void setDisplayValueString(std::string string) override;
  std::string getDisplayValueString() override;
  std::string getString() override;
};
