import DataFlow.csrc.pybind_module as _pym
import DataFlow.utils.api_export as api_export
from DataFlow.stream import InflateStreamMeta

@api_export(impl=_pym.DataDecompressor)
class DataDecompressor:
    """A class responsible for decompressing data streams."""
    def __init__(self, input):
        raise NotImplementedError("DataDecompressor is implemented in C++ extension.")
    
    def __iter__(self):
        raise NotImplementedError("__iter__ method is implemented in C++ extension.")
    
    @property
    def output_stream_meta(self) -> InflateStreamMeta:
        raise NotImplementedError("output_stream_meta property is implemented in C++ extension.")