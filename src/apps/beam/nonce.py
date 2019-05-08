from trezor import config
from trezor.crypto import beam

from apps.beam.helpers import (
    beam_app_id,
    get_master_nonce_idx,
)

def create_master_nonce():
    master_nonce = bytearray(32)
    beam.create_master_nonce(master_nonce)
    config.set(beam_app_id(), get_master_nonce_idx(), master_nonce)

def derive_nonce(idx):
    if idx != get_master_nonce_idx():
        old_nonce = config.get(beam_app_id(), idx)
        new_nonce = bytearray(32)
        new_image = bytearray(32)
        master_nonce = config.get(beam_app_id(), get_master_nonce_idx())
        beam.create_derived_nonce(master_nonce, new_nonce, new_image)
        config.set(beam_app_id(), idx, new_nonce)
        return (old_nonce, new_image)

def get_nonce(idx):
    old_nonce, _ = derive_nonce(idx)
    return old_nonce
