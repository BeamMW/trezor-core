# Automatically generated by pb2py
# fmt: off
import protobuf as p


class BeamGenerateNonce(p.MessageType):
    MESSAGE_WIRE_TYPE = 710

    def __init__(
        self,
        slot: int = None,
    ) -> None:
        self.slot = slot

    @classmethod
    def get_fields(cls):
        return {
            1: ('slot', p.UVarintType, 0),
        }