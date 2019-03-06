from trezor.crypto import beam
from trezor.messages.BeamSignedMessage import BeamSignedMessage
from trezor.messages.BeamSignature import BeamSignature

from apps.beam.helpers import (
    bin_to_str,
    get_beam_sk,
)
from apps.beam.layout import *


async def sign_message(ctx, msg):
    await require_confirm_sign_message(ctx, msg.msg, False)

    sign_nonce_pub_x = bytearray(32)
    sign_nonce_pub_y = bytearray(1)
    sign_k = bytearray(32)

    sk = get_beam_sk()
    beam.signature_sign(msg.msg, sk, sign_nonce_pub_x, sign_nonce_pub_y, sign_k)
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
