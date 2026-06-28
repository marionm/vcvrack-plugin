#include "GitHubTokenField.hpp"

#include <string>

void GitHubTokenField::draw(const DrawArgs& args) {
  size_t atPos = text.find('@');
  size_t splitIndex = (atPos == std::string::npos) ? 0 : atPos + 1;
  std::string usernameWithAt = text.substr(0, splitIndex);
  std::string token = text.substr(splitIndex);

  std::string originalText = text;
  text = usernameWithAt + std::string(token.size(), '*');
  TextField::draw(args);
  text = originalText;
}
