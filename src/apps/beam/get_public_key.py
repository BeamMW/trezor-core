from trezor.messages.BeamPublicKey import BeamPublicKey

from apps.common import layout
from apps.beam.helpers import (
    bin_to_str,
    get_beam_pk,
)


async def get_public_key(ctx, msg):
    pubkey = get_beam_pk()

    if msg.show_display:
        await layout.show_pubkey(ctx, pubkey)

    return BeamPublicKey(xpub=pubkey)
