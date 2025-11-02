import DataFlow.csrc.pybind_module as _pym
import DataFlow.utils.api_export as api_export

@api_export(impl=_pym.ByteStreamMeta)
class ByteStreamMeta:
    """ Metadata class for ByteStream data objects."""
    def __init__(self):
        raise NotImplementedError("ByteStreamMeta is implemented in C++ extension.")
    
    @property
    def data_type(self) -> str:
        raise NotImplementedError("data_type is implemented in C++ extension.")
    
@api_export(impl=_pym.ByteStream)
class ByteStream:
    """ ByteStream data object for handling raw byte streams."""
    def __init__(self, data: bytes, meta: ByteStreamMeta):
        raise NotImplementedError("ByteStream is implemented in C++ extension.")
    
    @property
    def data_meta(self) -> ByteStreamMeta:
        raise NotImplementedError("data_meta is implemented in C++ extension.")