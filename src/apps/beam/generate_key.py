from trezor.crypto import beam

from trezor.messages.BeamGenerateKey import BeamGenerateKey
from trezor.messages.BeamPublicKey import BeamPublicKey

from apps.common import storage


async def generate_key(ctx, msg):
    key_image_x = bytearray(32)
    key_image_y = bytearray(1)

    mnemonic = storage.get_mnemonic()
    seed = beam.phrase_to_seed(mnemonic)

    beam.generate_key(msg.kidv.idx, msg.kidv.type, msg.kidv.sub_idx, msg.kidv.value,
                      msg.is_coin_key, seed,
                      key_image_x, key_image_x)

    return BeamPublicKey(pub_x=key_image_x, pub_y=key_image_y)
