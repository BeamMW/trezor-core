from trezor.crypto import beam
from trezor.messages.BeamSignature import BeamSignature

from apps.common import storage

from apps.beam.layout import *

def BBS_KEY():
    return 1113748301


def bin_to_str(binary_data):
    return ''.join('{:02x}'.format(x) for x in binary_data)

def get_beam_kdf():
    # Get kdf
    mnemonic = storage.get_mnemonic()
    seed = beam.phrase_to_seed(mnemonic)
    seed_size = 32
    secret_key = bytearray(32)
    cofactor = bytearray(32)
    beam.seed_to_kdf(seed, seed_size, secret_key, cofactor)
    return (secret_key, cofactor)

def get_beam_sk():
    # Generate hash id
    a_id = 123456
    a_type = BBS_KEY()
    sub_idx = 0
    hash_id = bytearray(32)
    beam.generate_hash_id(a_id, a_type, sub_idx, hash_id)
    # Get kdf
    secret_key, cofactor = get_beam_kdf()
    # Derive key
    res_sk = bytearray(32)
    beam.derive_child_key(secret_key, 32, hash_id, 32, cofactor, res_sk)

    return res_sk

def get_beam_pk():
    # Secret key to public key
    public_key_x = bytearray(32)
    public_key_y = bytearray(1)
    sk = get_beam_sk()
    beam.secret_key_to_public_key(sk, public_key_x, public_key_y)

    return (public_key_x, public_key_y)

def is_valid_beam_message(signature, pk_x, pk_y, message):
    is_valid = beam.is_valid_signature(message,
                                       signature.nonce_pub_x, signature.nonce_pub_y,
                                       signature.sign_k, pk_x, pk_y)

    return is_valid
