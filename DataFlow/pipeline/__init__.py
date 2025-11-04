import DataFlow.csrc.pybind_module as _pym
import DataFlow.utils.api_export as api_export
from .data_reader import DataReader
from .data_decompressor import DataDecompressor

@api_export(impl=_pym.DataPipeline)
class DataPipeline:
    """A class representing a data processing pipeline."""
    @property
    def output_stream_meta(self):
        raise NotImplementedError("output_stream_meta property is implemented in C++ extension.")
    
    def __iter__(self):
        raise NotImplementedError("__iter__ method is implemented in C++ extension.")