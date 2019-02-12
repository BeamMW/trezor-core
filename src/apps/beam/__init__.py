from trezor import wire
from trezor.messages import MessageType

#from apps.common import HARDENED

def boot():
    #ns = [["secp256k1", HARDENED | 44, HARDENED | 128]]
    wire.add(MessageType.BeamDisplayMessage, __name__, 'display_message')
