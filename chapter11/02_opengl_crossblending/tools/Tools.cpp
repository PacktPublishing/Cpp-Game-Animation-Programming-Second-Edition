#include "Tools.h"

std::string Tools::getFilenameExt(std::string filename) {
  size_t pos = filename.find_last_of('.');
  if (pos != std::string::npos) {
    return filename.substr(pos + 1);
  }
  return std::string();
}
