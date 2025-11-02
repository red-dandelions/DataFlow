import DataFlow.csrc.pybind_module as _pym
import DataFlow.utils.api_export as api_export

@api_export(impl=_pym.InflateStreamMeta)
class InflateStreamMeta:
    """ Metadata class for InflateStream data objects."""
    def __init__(self):
        raise NotImplementedError("InflateStreamMeta is implemented in C++ extension.")
    
    @property
    def data_type(self) -> str:
        raise NotImplementedError("data_type is implemented in C++ extension.")
    

@api_export(impl=_pym.InflateStream)
class InflateStream:
    """ Metadata class for ByteStream data objects."""
    def __init__(self):
        raise NotImplementedError("InflateStream is implemented in C++ extension.")
    
    @property
    def data_meta(self) -> InflateStreamMeta:
        raise NotImplementedError("data_meta is implemented in C++ extension.")