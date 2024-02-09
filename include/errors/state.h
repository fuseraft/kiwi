#ifndef KIWI_ERRORS_STATE_H
#define KIWI_ERRORS_STATE_H

#include "error.h"

struct ErrorState {
  bool hasError = false;
  KiwiError error = KiwiError::create();

  ErrorState() {}

  void setError(const KiwiError& e) {
    error = e;
    hasError = true;
  }

  void clearError() {
    error = KiwiError::create();
    hasError = false;
  }

  bool isErrorSet() const { return hasError; }
};

#endif