# Automatically generated by pb2py
# fmt: off
import protobuf as p


class BeamSignedTransaction(p.MessageType):
    MESSAGE_WIRE_TYPE = 715

    def __init__(
        self,
        signature: bytes = None,
    ) -> None:
        self.signature = signature

    @classmethod
    def get_fields(cls):
        return {
            1: ('signature', p.BytesType, 0),
        }