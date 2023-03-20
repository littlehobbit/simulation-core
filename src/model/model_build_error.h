#ifndef __MODEL_BUILD_ERROR_H_HM0JTV0TWWPZ__
#define __MODEL_BUILD_ERROR_H_HM0JTV0TWWPZ__

#include <stdexcept>

namespace model {

class ModelBuildError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

}  // namespace model

#endif  // __MODEL_BUILD_ERROR_H_HM0JTV0TWWPZ__
