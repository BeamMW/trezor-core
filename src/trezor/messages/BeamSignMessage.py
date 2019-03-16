# Automatically generated by pb2py
# fmt: off
import protobuf as p


class BeamSignMessage(p.MessageType):
    MESSAGE_WIRE_TYPE = 702

    def __init__(
        self,
        msg: str = None,
        show_display: bool = None,
    ) -> None:
        self.msg = msg
        self.show_display = show_display

    @classmethod
    def get_fields(cls):
        return {
            1: ('msg', p.UnicodeType, 0),
            2: ('show_display', p.BoolType, 0),
        }