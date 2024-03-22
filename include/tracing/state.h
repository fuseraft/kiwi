#ifndef KIWI_ERRORS_STATE_H
#define KIWI_ERRORS_STATE_H

#include "error.h"

struct ErrorState {
  bool hasError = false;
  AstralError error = AstralError::create();

  ErrorState() {}

  void setError(const AstralError& e) {
    error = e;
    hasError = true;
  }

  void clearError() {
    error = AstralError::create();
    hasError = false;
  }

  bool isErrorSet() const { return hasError; }
};

#endif