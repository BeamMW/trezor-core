from trezor import wire
from trezor.messages import MessageType

from apps.common import HARDENED

def boot():
    ns = [["ed25519", HARDENED | 44, HARDENED | 1729]]
    wire.add(MessageType.BeamDisplayMessage, __name__, 'display_message')
