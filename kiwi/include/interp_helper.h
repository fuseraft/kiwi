#ifndef KIWI_INTERPHELPER_H
#define KIWI_INTERPHELPER_H

#include <vector>
#include "parsing/tokens.h"
#include "tracing/error.h"
#include "stackframe.h"
#include "web/httplib.h"

struct InterpHelper {
  static bool shouldUpdateFrameVariables(
      const k_string& varName,
      const std::shared_ptr<CallStackFrame> nextFrame) {
    return nextFrame->hasVariable(varName);
  }

  static void updateVariablesInCallerFrame(
      const std::unordered_map<k_string, k_value>& variables,
      std::shared_ptr<CallStackFrame> callerFrame) {
    auto& frameVariables = callerFrame->variables;
    for (const auto& var : variables) {
      if (shouldUpdateFrameVariables(var.first, callerFrame)) {
        frameVariables[var.first] = var.second;
      }
    }
  }

  static k_string getTemporaryId() {
    return "temporary_" + RNG::getInstance().random16();
  }

  static void updateListSlice(const Token& token, bool insertOp,
                              k_list& targetList, const SliceIndex& slice,
                              const k_list& rhsValues) {
    if (!std::holds_alternative<k_int>(slice.indexOrStart)) {
      throw IndexError(token, "Start index must be an integer.");
    } else if (!std::holds_alternative<k_int>(slice.stopIndex)) {
      throw IndexError(token, "Stop index must be an integer.");
    } else if (!std::holds_alternative<k_int>(slice.stepValue)) {
      throw IndexError(token, "Step value must be an integer.");
    }

    int start = std::get<k_int>(slice.indexOrStart);
    int stop = std::get<k_int>(slice.stopIndex);
    int step = std::get<k_int>(slice.stepValue);

    // This is a single element assignment.
    if (!slice.isSlice && insertOp) {
      stop = start;
    }

    auto& elements = targetList->elements;
    auto& rhsElements = rhsValues->elements;

    // Convert negative indices and adjust ranges
    int listSize = static_cast<int>(elements.size());
    int rhsSize = static_cast<int>(rhsElements.size());

    start += start < 0 ? listSize : 0;
    stop += stop < 0 ? listSize : 0;
    start = start < 0 ? 0 : start;
    stop = stop > listSize ? listSize : stop;
    // Special case for reverse slicing
    stop = step < 0 && stop == listSize ? -1 : stop;

    if (step == 1) {
      // Simple case: step is 1
      if (start >= stop) {
        elements.erase(elements.begin() + start, elements.begin() + stop);
        elements.insert(elements.begin() + start, rhsElements.begin(),
                        rhsElements.end());
      } else {
        std::copy(rhsElements.begin(), rhsElements.end(),
                  elements.begin() + start);
      }
    } else {
      // Complex case: step != 1
      int rhsIndex = 0;
      for (int i = start; i != stop && rhsIndex < rhsSize; i += step) {
        if ((step > 0 && i < listSize) || (step < 0 && i >= 0)) {
          elements[i] = rhsElements.at(rhsIndex++);
        } else {
          break;  // Avoid going out of bounds
        }
      }
    }
  }

  static k_value stringSlice(const Token& token, SliceIndex& slice,
                             const k_value& value) {
    auto string = std::get<k_string>(value);
    auto list = std::make_shared<List>();

    auto& elements = list->elements;
    elements.reserve(string.size());
    k_string temp(1, '\0');
    for (const char& c : string) {
      temp[0] = c;
      elements.emplace_back(temp);
    }

    auto sliced = listSlice(token, slice, list);

    if (std::holds_alternative<k_list>(sliced)) {
      auto slicedlist = std::get<k_list>(sliced)->elements;
      std::ostringstream sv;

      for (auto it = slicedlist.begin(); it != slicedlist.end(); ++it) {
        sv << Serializer::serialize(*it);
      }

      return sv.str();
    }

    return Serializer::serialize(sliced);
  }

  static k_value listSlice(const Token& token, const SliceIndex& slice,
                           const k_value& value) {
    auto list = std::get<k_list>(value);
    auto& elements = list->elements;

    if (!std::holds_alternative<k_int>(slice.indexOrStart)) {
      throw IndexError(token, "Start index must be an integer.");
    } else if (!std::holds_alternative<k_int>(slice.stopIndex)) {
      throw IndexError(token, "Stop index must be an integer.");
    } else if (!std::holds_alternative<k_int>(slice.stepValue)) {
      throw IndexError(token, "Step value must be an integer.");
    }

    int start = static_cast<int>(std::get<k_int>(slice.indexOrStart)),
        stop = static_cast<int>(std::get<k_int>(slice.stopIndex)),
        step = static_cast<int>(std::get<k_int>(slice.stepValue));

    // Adjust negative indices
    int listSize = static_cast<int>(elements.size());
    if (start < 0) {
      start = start + listSize > 0 ? start + listSize : 0;
    }

    if (stop < 0) {
      stop += listSize;
    } else {
      stop = stop < listSize ? stop : listSize;
    }

    // Adjust stop for reverse slicing
    if (step < 0 && stop == listSize) {
      stop = -1;
    }

    auto slicedList = std::make_shared<List>();
    auto& slicedElements = slicedList->elements;

    if (step < 0) {
      for (int i = (start == 0 ? listSize - 1 : start); i >= stop; i += step) {
        // Prevent out-of-bounds access
        if (i < 0 || i >= listSize) {
          break;
        }

        slicedElements.emplace_back(elements.at(i));
      }
    } else {
      for (int i = start; i < stop; i += step) {
        // Prevent out-of-bounds access
        if (i >= listSize) {
          break;
        }

        slicedElements.emplace_back(elements.at(i));
      }
    }
    return slicedList;
  }

  static std::vector<k_string> getWebServerEndpointList(const Token& term,
                                                        k_value& arg) {
    std::vector<k_string> endpointList;

    if (std::holds_alternative<k_string>(arg)) {
      endpointList.emplace_back(get_string(term, arg));
    } else if (std::holds_alternative<k_list>(arg)) {
      for (const auto& el : std::get<k_list>(arg)->elements) {
        if (std::holds_alternative<k_string>(el)) {
          auto endpoint = get_string(term, el);
          if (std::find(endpointList.begin(), endpointList.end(), endpoint) ==
              endpointList.end()) {
            endpointList.emplace_back(endpoint);
          }
        }
      }
    }

    return endpointList;
  }

  static k_hash getWebServerRequestHash(const httplib::Request& req) {
    auto requestHash = std::make_shared<Hash>();
    auto headers = req.headers;
    auto params = req.params;

    for (auto it = headers.begin(); it != headers.end(); ++it) {
      const auto& x = *it;
      requestHash->add(x.first, x.second);
    }

    auto pathParamsHash = std::make_shared<Hash>();
    for (const auto& pair : req.path_params) {
      pathParamsHash->add(pair.first, pair.second);
    }

    auto paramsHash = std::make_shared<Hash>();
    for (auto it = params.begin(); it != params.end(); ++it) {
      const auto& x = *it;
      paramsHash->add(x.first, x.second);
    }

    auto filesHash = std::make_shared<Hash>();

    for (const auto& file : req.files) {
      auto fileHash = std::make_shared<Hash>();
      fileHash->add("content", file.second.content);
      fileHash->add("content_type", file.second.content_type);
      fileHash->add("filename", file.second.filename);
      fileHash->add("name", file.second.name);
      filesHash->add(file.first, fileHash);
    }

    requestHash->add("body", req.body);
    requestHash->add("files", filesHash);
    requestHash->add("path", req.path);
    requestHash->add("path_params", pathParamsHash);
    requestHash->add("params", paramsHash);

    return requestHash;
  }
};

#endif