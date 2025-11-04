/**
 * @file macros.h
 * @brief Common macros definitions.
 *
 * Author: Jasmine (1011694931@qq.com)
 * Created on: 2025-11-3
 *
 * Copyright (c) 2025 Jasmine. All rights reserved.
 */

#pragma once

#include "functions.h"

namespace data_flow {

#define DISABLE_COPY_AND_ASSIGN(ClassName) \
  ClassName(const ClassName&) = delete;    \
  ClassName& operator=(const ClassName&) = delete;

#define DISABLE_MOVE_AND_ASSIGN(ClassName) \
  ClassName(ClassName&&) = delete;         \
  ClassName& operator=(ClassName&&) = delete;

}  // namespace data_flow