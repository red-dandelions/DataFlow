import DataFlow.csrc.pybind_module as _pym
import DataFlow.utils.api_export as api_export

@api_export(impl=_pym.StringStreamMeta)
class StringStreamMeta:
    """ Metadata class for StringStream data objects."""
    def __init__(self):
        raise NotImplementedError("StringStreamMeta is implemented in C++ extension.")
    
    @property
    def stream_type_name(self) -> str:
        raise NotImplementedError("stream_type_name is implemented in C++ extension.")
    

@api_export(impl=_pym.StringStream)
class StringStream:
    def __init__(self):
        raise NotImplementedError("StringStream is implemented in C++ extension.")
    
    @property
    def stream_meta(self) -> StringStreamMeta:
        raise NotImplementedError("stream_meta is implemented in C++ extension.")

    @property
    def value(slef) -> str:
        raise NotImplementedError("value is implemented in C++ extension.")