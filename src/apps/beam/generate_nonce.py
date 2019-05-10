from trezor.messages.BeamGenerateNonce import BeamGenerateNonce
from trezor.messages.BeamECCImage import BeamECCImage
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

    _, nonce_image = derive_nonce(msg.slot)
    return BeamECCImage(image_x=nonce_image)
