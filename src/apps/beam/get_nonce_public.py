from trezor.messages.BeamGenerateNonce import BeamGenerateNonce
from trezor.messages.BeamECCImage import BeamECCImage
from trezor.messages.Failure import Failure

from apps.beam.nonce import *
from apps.beam.helpers import (
    get_master_nonce_idx,
)

async def get_nonce_public(ctx, msg):
    idx = msg.slot
    if idx == get_master_nonce_idx() or idx > 255:
        return Failure(message='Incorrect slot provided')

    if not is_master_nonce_created():
        return Failure(message='Nonce Generator is not initialized')

    pubkey_x, pubkey_y = get_nonce_pub(msg.slot)
    return BeamPublicKey(pub_x=pubkey_x, pub_y=pubkey_y)
