from trezor import wire
from trezor.messages import MessageType

#from apps.common import HARDENED

def boot():
    #ns = [["secp256k1", HARDENED | 44, HARDENED | 128]]
    wire.add(MessageType.BeamDisplayMessage, __name__, 'display_message')
    wire.add(MessageType.BeamSignMessage, __name__, 'sign_message')
    wire.add(MessageType.BeamVerifyMessage, __name__, 'verify_message')
    wire.add(MessageType.BeamGetPublicKey, __name__, 'get_public_key')
