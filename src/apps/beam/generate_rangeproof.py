from trezor.crypto import beam

from trezor.messages.BeamGenerateRangeproof import BeamGenerateRangeproof
from trezor.messages.BeamKeyIDV import BeamKeyIDV
from trezor.messages.BeamRangeproofData import BeamRangeproofData

from apps.common import storage


async def generate_rangeproof(ctx, msg):
    asset_id = bytearray(0)
    #TODO: calc num bytes in rangeproof
    rangeproof_data = bytearray(700)

    mnemonic = storage.get_mnemonic()
    seed = beam.phrase_to_seed(mnemonic)

    beam.generate_rp_from_key_idv(msg.kidv.idx, msg.kidv.type, msg.kidv.sub_idx, msg.kidv.value,
                                  asset_id, msg.is_public, seed,
                                  rangeproof_data)

    return BeamRangeproofData(data=rangeproof_data, is_public=msg.is_public)
