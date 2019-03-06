from trezor.crypto import beam
from trezor.messages.Success import Success
from trezor.messages.BeamVerifyMessage import BeamVerifyMessage
from trezor.messages.BeamSignature import BeamSignature

from apps.beam.helpers import (
    bin_to_str,
    get_beam_sk,
    is_valid_beam_message,
)
from apps.beam.layout import *

async def verify_message(ctx, msg):
    message = str(msg.message, 'utf-8')
    if len(msg.signature.nonce_pub_x) != 32 \
       or len(msg.signature.nonce_pub_y) != 1 \
       or len(msg.signature.sign_k) != 32:
        raise wire.DataError("Invalid signature")

    is_valid = is_valid_beam_message(msg.signature, message)

    if not is_valid:
        raise wire.DataError("Invalid signature")

    is_valid_msg = 'Sign_x: {}; Sign_y: {}; Sign_k: {}; Is valid: {}'.format(bin_to_str(msg.signature.nonce_pub_x),
                                                                             bin_to_str(msg.signature.nonce_pub_y),
                                                                             bin_to_str(msg.signature.sign_k),
                                                                             is_valid)
    await require_validate_sign_message(ctx, is_valid_msg)

    return Success(message="Message verified")

