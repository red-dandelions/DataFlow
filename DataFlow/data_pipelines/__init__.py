import DataFlow.csrc.pybind_module as _pym
import DataFlow.utils.api_export as api_export

@api_export(impl=_pym.DataPipeline)
class DataPipeline:
    """A class representing a data processing pipeline."""
    @property
    def output_data_meta(self):
        raise NotImplementedError("output_data_meta property is implemented in C++ extension.")
    
    def __iter__(self):
        raise NotImplementedError("__iter__ method is implemented in C++ extension.")