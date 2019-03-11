from trezor.messages.BeamPublicKey import BeamPublicKey

from apps.common import layout
from apps.beam.helpers import (
    bin_to_str,
    get_beam_pk,
)


async def get_public_key(ctx, msg):
    pubkey_x, pubkey_y = get_beam_pk()

    if msg.show_display:
        await layout.show_pubkey(ctx, pubkey_x)
    if msg.show_display:
        await layout.show_pubkey(ctx, pubkey_y)

    return BeamPublicKey(pub_x=pubkey_x, pub_y=pubkey_y)
