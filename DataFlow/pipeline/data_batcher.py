import DataFlow.csrc.pybind_module as _pym
import DataFlow.utils.api_export as api_export
from DataFlow.stream import BatchMeta

@api_export(impl=_pym.DataBatcher)
class DataBatcher:
    # filter_fns = [(fn, args), (fn, args), ...]
    def __init__(self, input, batch_size, filter_fns, drop_last_batch = True):
        raise NotImplementedError("DataBatcher is implemented in C++ extension.")
    
    def __iter__(self):
        raise NotImplementedError("__iter__ method is implemented in C++ extension.")
    
    @property
    def output_stream_meta(self) -> BatchMeta:
        raise NotImplementedError("output_stream_meta property is implemented in C++ extension.")