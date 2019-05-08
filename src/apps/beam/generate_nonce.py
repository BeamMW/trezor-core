from trezor.messages.BeamGenerateNonce import BeamGenerateNonce
from trezor.messages.BeamECCImage import BeamECCImage
from trezor.messages.Failure import Failure

from apps.beam.nonce import *
from apps.beam.helpers import (
    get_master_nonce_idx,
)

async def generate_nonce(ctx, msg):
    idx = msg.slot
    if i == get_master_nonce_idx() or i > 255:
        return Failure(message='Incorrect slot provided')

    _, nonce_image = derive_nonce(msg.slot)
    return BeamECCImage(image_x=nonce_image)
