import DataFlow.csrc.pybind_module as _pym
import DataFlow.utils.api_export as api_export
from DataFlow.stream import ByteStreamMeta

@api_export(impl=_pym.DataReader)
class DataReader:
    """A class responsible for reading data from various sources."""
    def __init__(self, input, source: str):
        raise NotImplementedError("DataReader is implemented in C++ extension.")
    
    def __iter__(self):
        raise NotImplementedError("__iter__ method is implemented in C++ extension.")
    
    @property
    def output_stream_meta(self) -> ByteStreamMeta:
        raise NotImplementedError("output_stream_meta property is implemented in C++ extension.")