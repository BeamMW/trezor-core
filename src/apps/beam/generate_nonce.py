from trezor.messages.BeamGenerateNonce import BeamGenerateNonce
from trezor.messages.BeamPublicKey import BeamPublicKey
from trezor.messages.Failure import Failure

from apps.beam.nonce import *
from apps.beam.helpers import (
    get_master_nonce_idx,
)

async def generate_nonce(ctx, msg):
    idx = msg.slot
    if idx == get_master_nonce_idx() or idx > 255:
        return Failure(message='Incorrect slot provided')

    if not is_master_nonce_created():
        return Failure(message='Nonce Generator is not initialized')

    _, new_nonce = create_nonce(idx)
    pubkey_x, pubkey_y = calc_nonce_pub(new_nonce)
    return BeamPublicKey(pub_x=pubkey_x, pub_y=pubkey_y)
