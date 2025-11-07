import DataFlow.csrc.pybind_module as _pym
import DataFlow.utils.api_export as api_export
from .byte_stream import ByteStreamMeta, ByteStream
from .inflate_stream import InflateStreamMeta, InflateStream
from .batch_row import DenseColumn, SparseColumn, BatchRowMeta, BatchRow

@api_export(impl=_pym.StreamMeta)
class StreamMeta:
    def __init__(self):
        raise NotImplementedError("StreamMeta is an abstract class and cannot be instantiated directly.")
    
    @property
    def stream_type_name(self):
        raise NotImplementedError("Subclasses must implement the stream_type_name property.")
    
@api_export(impl=_pym.Stream)
class Stream:
    def __init__(self):
        raise NotImplementedError("Stream is an abstract class and cannot be instantiated directly.")
    
    @property
    def stream_meta(self):
        raise NotImplementedError("Subclasses must implement the stream_meta property.")