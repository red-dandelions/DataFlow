import DataFlow.csrc.pybind_module as _pym
import DataFlow.utils.api_export as api_export
import numpy as np
from typing import List

@api_export(impl=_pym.Column)
class Column:
    """ Base class for columns in BatchRow data objects."""
    def __init__(self, name, dtype, shape, column_type):
        raise NotImplementedError("Column is implemented in C++ extension.")
    
    @property
    def name(self):
        raise NotImplementedError("name is implemented in C++ extension.")
    
    @property
    def dtype(self):
        raise NotImplementedError("dtype is implemented in C++ extension.")

    @property
    def shape(self):
        raise NotImplementedError("shape is implemented in C++ extension.")
    
    @property
    def column_type(self):
        raise NotImplementedError("column_type is implemented in C++ extension.")

def DenseColumn(name, shape, dtype=np.float32):
    return Column(name, dtype, shape, column_type=_pym.ColumnType.dense)

def SparseColumn(name, shape, dtype=np.int64):
    return Column(name, dtype, shape, column_type=_pym.ColumnType.sparse)

    def __init__(self, columns: List[Column]):
        raise NotImplementedError("BatchRow is implemented in C++ extension.")
    
    @property
    def stream_meta(self) -> BatchRowMeta:
        raise NotImplementedError("stream_meta is implemented in C++ extension.")

@api_export(impl=_pym.BatchRowMeta)
class BatchRowMeta:
    def __init__(self, columns):
        raise NotImplementedError("BatchRowMeta is implemented in C++ extension.")
    
    @property
    def stream_type_name(self) -> str:
        raise NotImplementedError("stream_type_name is implemented in C++ extension.")
    
@api_export(impl=_pym.BatchRow)
class BatchRow:
    def __init__(self, batch_row_meta):
        raise NotImplementedError("BatchRow is implemented in C++ extension.")
    
    @property
    def stream_meta(self) -> BatchRowMeta:
        raise NotImplementedError("stream_meta is implemented in C++ extension.")