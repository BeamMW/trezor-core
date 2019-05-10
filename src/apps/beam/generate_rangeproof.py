from trezor.crypto import beam

from trezor.messages.BeamGenerateRangeproof import BeamGenerateRangeproof
from trezor.messages.BeamKeyIDV import BeamKeyIDV
from trezor.messages.BeamRangeproofData import BeamRangeproofData

async def generate_rangeproof(ctx, msg):
    #TODO: request nonce for given slot number
    slot = msg.slot

    #TODO: pass real sk and nonce here
    sk = bytearray(0)
    nonce = bytearray(0)
    asset_id = bytearray(0)
    #TODO: calc num bytes in rangeproof
    rangeproof_data = bytearray(700)

    print("Got message!: " + str(msg))
    beam.generate_rp_from_key_idv(msg.kidv.idx, msg.kidv.type, msg.kidv.sub_idx, msg.kidv.value,
                                  sk, nonce, asset_id, msg.is_public,
                                  rangeproof_data)

    return BeamRangeproofData(data=rangeproof_data, is_public=msg.is_public)
