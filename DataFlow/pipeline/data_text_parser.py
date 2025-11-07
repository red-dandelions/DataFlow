import DataFlow.csrc.pybind_module as _pym
import DataFlow.utils.api_export as api_export
from DataFlow.stream import BatchRowMeta, DenseColumn, SparseColumn
from typing import List

@api_export(impl=_pym.DataTextParser)
class DataTextParser:
    """A class responsible for reading data from various sources."""
    def __init__(self, input, format: str, columns: List[DenseColumn or SparseColumn], field_delim: str = '|'):
        raise NotImplementedError("DataTextParser is implemented in C++ extension.")
    
    def __iter__(self):
        raise NotImplementedError("__iter__ method is implemented in C++ extension.")
    
    @property
    def output_stream_meta(self) -> BatchRowMeta:
        raise NotImplementedError("output_stream_meta property is implemented in C++ extension.")