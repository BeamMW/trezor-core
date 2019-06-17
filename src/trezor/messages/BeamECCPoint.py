# Automatically generated by pb2py
# fmt: off
import protobuf as p


class BeamECCPoint(p.MessageType):
    MESSAGE_WIRE_TYPE = 711

    def __init__(
        self,
        x: bytes = None,
        y: bool = None,
    ) -> None:
        self.x = x
        self.y = y

    @classmethod
    def get_fields(cls):
        return {
            1: ('x', p.BytesType, 0),
            2: ('y', p.BoolType, 0),
        }