import DataFlow.csrc.pybind_module as _pym
import DataFlow.utils.api_export as api_export
from .byte_stream import ByteStreamMeta, ByteStream
from .inflate_stream import InflateStreamMeta, InflateStream


@api_export(impl=_pym.DataObjectMeta)
class DataObjectMeta:
    def __init__(self):
        raise NotImplementedError("DataObject is an abstract class and cannot be instantiated directly.")
    
    @property
    def data_type(self):
        raise NotImplementedError("Subclasses must implement the data_type property.")
    
@api_export(impl=_pym.DataObject)
class DataObject:
    def __init__(self):
        raise NotImplementedError("DataObject is an abstract class and cannot be instantiated directly.")
    
    @property
    def data_meta(self):
        raise NotImplementedError("Subclasses must implement the data_meta property.")