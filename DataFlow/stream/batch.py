import DataFlow.csrc.pybind_module as _pym
import DataFlow.utils.api_export as api_export

@api_export(impl=_pym.BatchMeta)
class BatchMeta:
    def __init__(self, columns):
        raise NotImplementedError("BatchMeta is implemented in C++ extension.")
    
    @property
    def stream_type_name(self) -> str:
        raise NotImplementedError("stream_type_name is implemented in C++ extension.")
    
@api_export(impl=_pym.Batch)
class Batch:
    def __init__(self, batch_meta):
        raise NotImplementedError("Batch is implemented in C++ extension.")
    
    @property
    def stream_meta(self) -> BatchMeta:
        raise NotImplementedError("stream_meta is implemented in C++ extension.")

    def __str__(self) -> str:
        raise NotImplementedError("__str__ is implemented in C++ extension.")