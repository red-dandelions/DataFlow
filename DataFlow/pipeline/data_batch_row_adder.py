import DataFlow.csrc.pybind_module as _pym
import DataFlow.utils.api_export as api_export
from DataFlow.stream import BatchRowMeta

@api_export(impl=_pym.DataBatchRowAdder)
class DataBatchRowAdder:
    def __init__(self, input, args, add_columns):
        raise NotImplementedError("DataBatchRowAdder is implemented in C++ extension.")
    
    def __iter__(self):
        raise NotImplementedError("__iter__ method is implemented in C++ extension.")
    
    @property
    def output_stream_meta(self) -> BatchRowMeta:
        raise NotImplementedError("output_stream_meta property is implemented in C++ extension.")