/**
 * @file data_pipeline_iterator.cc
 * @brief Implementation of DataPipeline iterator for Python.
 *
 * Author: Jasmine (1011694931@qq.com)
 * Created on: 2025-11-1
 *
 * Copyright (c) 2025 Jasmine. All rights reserved.
 */

#include "Python.h"

#include <memory>

#include "pybind11/pybind11.h"

#include "data_pipeline.h"

namespace {
using data_flow::DataPipeline;
/**
 * @brief Python iterator object for DataPipeline.
 */
struct DataPipelineIterator {
  PyObject_HEAD;
  std::shared_ptr<DataPipeline> data_pipeline;
};

/**
 * @brief Retrieve the next item from the DataPipeline iterator.
 * @param self Pointer to the DataPipelineIterator object.
 * @return PyObject* representing the next item, or nullptr on error or end of iteration.
 */
PyObject* DataPipelineIterator_next(DataPipelineIterator* self) {
  auto status_or_stream = self->data_pipeline->next();
  if (!status_or_stream.ok()) {
    PyErr_SetString(PyExc_RuntimeError, status_or_stream.status().message().data());
    return nullptr;
  }

  auto stream = status_or_stream.value();
  if (stream == nullptr) {
    PyErr_SetNone(PyExc_StopIteration);
    return nullptr;
  }

  return self->data_pipeline->as_python_object(stream);
}

/**
 * @brief Python type object for DataPipelineIterator.
 * @return Pointer to the PyTypeObject representing DataPipelineIterator.
 */
PyTypeObject* GetIterType() {
  static auto type_getter = []() -> PyTypeObject* {
    static PyTypeObject DataPipelineIterator_Type = {
        PyVarObject_HEAD_INIT(nullptr, 0),
        "DataPipelineIterator",       /* tp_name */
        sizeof(DataPipelineIterator), /* tp_basicsize */
        0,                            /* tp_itemsize */
        [](PyObject* self) -> void {
          auto iter = reinterpret_cast<DataPipelineIterator*>(self);
          iter->data_pipeline.~shared_ptr();
          Py_TYPE(self)->tp_free(self);
        },                                             /* tp_dealloc */
        0,                                             /* tp_vectorcall_offset */
        nullptr,                                       /* tp_getattr */
        nullptr,                                       /* tp_setattr */
        nullptr,                                       /* tp_reserved */
        nullptr,                                       /* tp_repr */
        nullptr,                                       /* tp_as_number */
        nullptr,                                       /* tp_as_sequence */
        nullptr,                                       /* tp_as_mapping */
        nullptr,                                       /* tp_hash  */
        nullptr,                                       /* tp_call */
        nullptr,                                       /* tp_str */
        nullptr,                                       /* tp_getattro */
        nullptr,                                       /* tp_setattro */
        nullptr,                                       /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_FINALIZE, /* tp_flags */
        "DataPipeline iterator",                       /* tp_doc */
        nullptr,                                       /* tp_traverse */
        nullptr,                                       /* tp_clear */
        nullptr,                                       /* tp_richcompare */
        0,                                             /* tp_weaklistoffset */
        [](PyObject* self) -> PyObject* {
          Py_INCREF(self);
          return self;
        },                                                                       /* tp_iter */
        (iternextfunc)DataPipelineIterator_next,                                 /* tp_iternext */
        nullptr,                                                                 /* tp_methods */
        nullptr,                                                                 /* tp_members */
        nullptr,                                                                 /* tp_getset */
        nullptr,                                                                 /* tp_base */
        nullptr,                                                                 /* tp_dict */
        nullptr,                                                                 /* tp_descr_get */
        nullptr,                                                                 /* tp_descr_set */
        0,                                                                       /* tp_dictoffset */
        [](PyObject* self, PyObject* args, PyObject* kwds) -> int { return 0; }, /* tp_init */
        nullptr,                                                                 /* tp_alloc */
        [](PyTypeObject* type, PyObject* args, PyObject* kwds) -> PyObject* {
          if (type) {
            return type->tp_alloc(type, 0);
          }
          return nullptr;
        },                             /* tp_new */
        nullptr,                       /* tp_free */
        nullptr,                       /* tp_is_gc */
        nullptr,                       /* tp_bases */
        nullptr,                       /* tp_mro */
        nullptr,                       /* tp_cache */
        nullptr,                       /* tp_subclasses */
        nullptr,                       /* tp_weaklist */
        nullptr,                       /* tp_del */
        0,                             /* tp_version_tag */
        [](PyObject* self) -> void {}, /* tp_finalize */
        nullptr                        /* tp_vectorcall */
    };

    if (PyType_Ready(&DataPipelineIterator_Type) < 0) {
      LOG(ERROR) << "Failed to ready DataPipelineIterator_Type";
      return nullptr;
    }
    return &DataPipelineIterator_Type;
  };

  static PyTypeObject* type = type_getter();

  return type;
}

/**
 * @brief Create a new DataPipeline iterator instance.
 * @return PyObject* representing the new iterator.
 */
inline PyObject* GetIterator() {
  auto iter = PyObject_CallObject(reinterpret_cast<PyObject*>(GetIterType()), nullptr);

  DATAFLOW_THROW_IF(PyErr_Occurred != nullptr,
                    pybind11::str(pybind11::handle(PyErr_Occurred())).cast<std::string>());

  return iter;
}
}  // namespace

namespace data_flow {
PyObject* GetDataPipelineIterator(std::shared_ptr<DataPipeline> pipeline) {
  auto iter = GetIterator();

  auto p = reinterpret_cast<DataPipelineIterator*>(iter);
  p->data_pipeline = pipeline;

  return iter;
}
}  // namespace data_flow