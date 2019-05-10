from trezor import config
from trezor.crypto import beam

from apps.beam.helpers import (
    beam_app_id,
    get_master_nonce_idx,
)

def create_master_nonce(seed):
    master_nonce = bytearray(32)
    beam.create_master_nonce(master_nonce, seed)
    config.set(beam_app_id(), get_master_nonce_idx(), master_nonce)

def is_master_nonce_created():
    master_nonce = config.get(beam_app_id(), get_master_nonce_idx())
    return len(master_nonce) > 0

def derive_nonce(idx):
    if idx != get_master_nonce_idx():
        old_nonce = config.get(beam_app_id(), idx)
        new_nonce = bytearray(old_nonce) if len(old_nonce) > 0 else bytearray(32)
        new_image = bytearray(32)
        master_nonce = config.get(beam_app_id(), get_master_nonce_idx())
        beam.create_derived_nonce(master_nonce, idx, new_nonce, new_image)
        config.set(beam_app_id(), idx, new_nonce)
        return (old_nonce, new_image)

def get_nonce(idx):
    old_nonce, _ = derive_nonce(idx)
    return old_nonce
