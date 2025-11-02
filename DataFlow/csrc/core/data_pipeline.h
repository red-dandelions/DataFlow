/**
 * @file data_pipeline.h
 * @brief Definition of DataPipeline class and related functions.
 *
 * Author: Jasmine (1011694931@qq.com)
 * Created on: 2025-11-1
 *
 * Copyright (c) 2025 Jasmine. All rights reserved.
 */

#pragma once

#include "absl/status/statusor.h"

#include "data_object.h"

// 前向声明，避免在头文件包含 Python.h
struct _object;
using PyObject = _object;

namespace data_flow {

/**
 * @brief DataPipeline is an abstract base class representing a data processing pipeline.
 */
struct DataPipeline : std::enable_shared_from_this<DataPipeline> {
  virtual ~DataPipeline() = default;

  /**
   * @brief return the metadata of the output DataObject produced by this pipeline.
   */
  virtual std::shared_ptr<DataObjectMeta> output_data_meta() const = 0;

  /**
   * @brief retrieve the next DataObject from the pipeline.
   * @return StatusOr containing the next DataObject, or an error status if retrieval fails, or
   * nullptr if the pipeline is exhausted.
   */
  virtual absl::StatusOr<std::shared_ptr<DataObject>> next() = 0;

  /**
   * @brief Convert a DataObject to a Python object.
   * @param data_object Shared pointer to the DataObject to convert.
   * @return PyObject* representing the Python object.
   */
  virtual PyObject* as_python_object(std::shared_ptr<DataObject> data_object) const = 0;
};

/**
* @brief Create a Python iterator for the given DataPipeline.

* @param pipeline Shared pointer to the DataPipeline instance.
* @return PyObject* representing the Python iterator.
*/
PyObject* GetDataPipelineIterator(std::shared_ptr<DataPipeline> pipeline);
}  // namespace data_flow