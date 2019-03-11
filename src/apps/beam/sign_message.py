from trezor.crypto import beam
from trezor.crypto.hashlib import sha256
from trezor.utils import HashWriter

from trezor.messages.BeamSignedMessage import BeamSignedMessage
from trezor.messages.BeamSignature import BeamSignature

from apps.beam.helpers import (
    bin_to_str,
    get_beam_sk,
)
from apps.beam.layout import *

def message_digest(message):
    h = HashWriter(sha256())
    signed_message_header = 'Beam Signed Message:\n'
    h.extend(signed_message_header)
    h.extend(str(len(message)))
    h.extend(message)
    return sha256(h.get_digest()).digest()
    #return h.get_digest()


async def sign_message(ctx, msg):
    await require_confirm_sign_message(ctx, msg.msg, False)

    sign_nonce_pub_x = bytearray(32)
    sign_nonce_pub_y = bytearray(1)
    sign_k = bytearray(32)

    sk = get_beam_sk()
    digest = message_digest(msg.msg)
    beam.signature_sign(digest, sk, sign_nonce_pub_x, sign_nonce_pub_y, sign_k)
    is_valid_msg = 'Sign_x: {}; Sign_y: {}; Sign_k: {}'.format(sign_nonce_pub_x,
                                                               sign_nonce_pub_y,
                                                               sign_k)

    if msg.show_display:
        while True:
            if await require_validate_sign_message(ctx, is_valid_msg):
                break

    signature = BeamSignature(nonce_pub_x=sign_nonce_pub_x, nonce_pub_y=sign_nonce_pub_y, sign_k=sign_k)

    res = BeamSignedMessage(signature)
    return res
