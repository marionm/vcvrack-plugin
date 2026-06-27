#include <rack.hpp>

#include <string>

struct ScaleParamQuantity : rack::ParamQuantity {
  std::string getString() override;
};
