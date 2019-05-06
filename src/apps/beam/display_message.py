from trezor import ui
from trezor import config
from trezor.crypto import beam
from trezor.messages import ButtonRequestType
from trezor.messages.BeamConfirmResponseMessage import BeamConfirmResponseMessage
from trezor.ui.text import Text

#from apps.common.layout import show_qr
from apps.common.confirm import *
from apps.common.layout import *

from apps.beam.helpers import (
    bin_to_str,
)
from apps.beam.layout import *

#TEST
from apps.common import storage
from trezor.crypto import random
from apps.beam.helpers import get_beam_pk
from apps.beam.sign_message import sign_message
from apps.beam.verify_message import verify_message
from trezor.messages.BeamPublicKey import BeamPublicKey
import utime

def hex_str_to_bytearray(hex_data, name='', print_info=False):
    import ubinascii
    #if hex_data.startswith('0x'):
    #    hex_data = hex_data[2:]
    #    if print_info:
    #        print('Converted {}: {}'.format(name, hex_data))

    #return bytearray.fromhex(hex_data)
    res = ubinascii.unhexlify(hex_data.strip())
    res = bytearray(hex_data)#, 'utf-8')
    print(res)
    return res

async def display_message(ctx, msg):
    ### TEST BEAM hello_crypto_world
    text_to_send_back = msg.text + ' This is a message from device! Received code:' + str(beam.hello_crypto_world())
    res = BeamConfirmResponseMessage(text=text_to_send_back, response=True)
    print(res.text)
    return res

def test_owner_key(ctx, msg):
    owner_key = config.get(28, 1)
    text_to_send_back = 'Owner key : {}'.format(str(owner_key, 'utf-8'))
    print(text_to_send_back)
    await require_validate_sign_message(ctx, str(owner_key, 'utf-8'))

    text = Text(msg.text, ui.ICON_SEND, icon_color=ui.GREEN)
    text.bold("BEAM")
    print("Received new message call:")
    print(msg.show_display)

    msg.msg = "Hello, world!"
    msg.show_display = False

    start = utime.ticks_ms()
    for i in range(100):
        sign_msg =  await sign_message(ctx, msg)
    #end = pyb.millis()
    end = utime.ticks_ms()
    sign_time = end - start
    text = 'Time elapsed: {} ms'.format(str(end-start))
    await beam_confirm_message(ctx, 'Bench', text, False)

    pk_x, pk_y = get_beam_pk()
    public_key = BeamPublicKey(pub_x=pk_x, pub_y=pk_y)
    msg.message = msg.msg
    msg.public_key = public_key
    msg.signature = sign_msg.signature
    start = utime.ticks_ms()
    for i in range(1000):
        await verify_message(ctx, msg)
    #end = pyb.millis()
    end = utime.ticks_ms()
    verify_time = end - start
    text = 'Time elapsed: {} ms'.format(str(end-start))
    await beam_confirm_message(ctx, 'Bench', text, False)

    text_to_send_back = 'Sign time elapsed: {} ms; Verify time elapsed: {} ms'.format(str(sign_time), str(verify_time))

    res = BeamConfirmResponseMessage(text=text_to_send_back, response=True)
    print(res.text)
    return res

async def test_storage(ctx, msg):
    #start = pyb.millis()
    start = utime.ticks_ms()
    value = random.bytes(256)
    for i in range(4):
        config.set(28, i, value)
    #end = pyb.millis()
    end = utime.ticks_ms()

    start = utime.time()
    for i in range(4):
        print(bin_to_str(config.get(28, i)))
    end = utime.time()
    ellapsed_ms = (end-start) / 10000
    print('Time elapsed: {}'.format(str(ellapsed_ms)))
    await require_validate_sign_message(ctx, str(ellapsed_ms))

async def test_beam_phrase_to_seed():
    ### TEST BEAM phrase_to_seed
    mnemonic = storage.get_mnemonic()
    seed = beam.phrase_to_seed(mnemonic)
    seed_str = ''.join('{:02x}'.format(x) for x in seed)
    text_to_send_back = 'Mnemonic: ' + mnemonic + '\nSeed: ' + seed_str

async def test_beam_generate_hash_id():
    ### TEST BEAM generate_hash_id
    a_id = 123456
    a_type = 1113748301
    sub_idx = 0
    out32 = bytearray(32)
    beam.generate_hash_id(a_id, a_type, sub_idx, out32)

    out_str = ''.join('{:02x}'.format(x) for x in out32)
    text_to_send_back = 'Generated hash id: ' + out_str

async def test_beam_seed_to_kdf():
    ### TEST BEAM seed_to_kdf
    mnemonic = storage.get_mnemonic()
    seed = beam.phrase_to_seed(mnemonic)
    seed_size = 32
    out_gen32 = bytearray(32)
    out_cofactor = bytearray(32)
    beam.seed_to_kdf(seed, seed_size, out_gen32, out_cofactor)

    gen32_str = ''.join('{:02x}'.format(x) for x in out_gen32)
    cofactor_str = ''.join('{:02x}'.format(x) for x in out_cofactor)
    text_to_send_back = 'Gen32: {}\nCofactor: {}'.format(gen32_str, cofactor_str)

async def test_beam_derive_key():
    ### TEST BEAM derive_key
    ## Get kdf
    mnemonic = storage.get_mnemonic()
    seed = beam.phrase_to_seed(mnemonic)
    seed_size = 32
    secret_key = bytearray(32)
    cofactor = bytearray(32)
    beam.seed_to_kdf(seed, seed_size, secret_key, cofactor)

    ## Generate hash id
    a_id = 123456
    a_type = 1113748301
    sub_idx = 0
    hash_id = bytearray(32)
    beam.generate_hash_id(a_id, a_type, sub_idx, hash_id)

    ## Derive key
    out_res_sk = bytearray(32)
    #print('SK: {}\nHash_id: {}'.format(bin_to_str(secret_key), bin_to_str(hash_id)))
    beam.derive_child_key(secret_key, 32, hash_id, 32, cofactor, out_res_sk)

    out_res_sk_str = bin_to_str(out_res_sk)
    text_to_send_back = 'Res_sk: {}'.format(out_res_sk_str)

async def test_beam_sk_to_pk():
    ### TEST BEAM sk_to_pk
    ## Get kdf
    mnemonic = storage.get_mnemonic()
    seed = beam.phrase_to_seed(mnemonic)
    seed_size = 32
    secret_key = bytearray(32)
    cofactor = bytearray(32)
    beam.seed_to_kdf(seed, seed_size, secret_key, cofactor)

    ## Generate hash id
    a_id = 123456
    a_type = 1113748301
    sub_idx = 0
    hash_id = bytearray(32)
    beam.generate_hash_id(a_id, a_type, sub_idx, hash_id)

    ## Derive key
    res_sk = bytearray(32)
    beam.derive_child_key(secret_key, 32, hash_id, 32, cofactor, res_sk)

    ## Secret key to public key
    #public_key = bytearray(32)
    #beam.secret_key_to_public_key(res_sk, public_key)
    #text_to_send_back = 'Public key: {}'.format(bin_to_str(public_key))

async def test_beam_signature_sign():
    ### TEST BEAM signature_sign
    ## Get kdf
    mnemonic = storage.get_mnemonic()
    seed = beam.phrase_to_seed(mnemonic)
    seed_size = 32
    secret_key = bytearray(32)
    cofactor = bytearray(32)
    beam.seed_to_kdf(seed, seed_size, secret_key, cofactor)

    ## Generate hash id
    a_id = 123456
    a_type = 1113748301
    sub_idx = 0
    hash_id = bytearray(32)
    beam.generate_hash_id(a_id, a_type, sub_idx, hash_id)

    ## Derive key
    res_sk = bytearray(32)
    beam.derive_child_key(secret_key, 32, hash_id, 32, cofactor, res_sk)

    ## Secret key to public key
    public_key = bytearray(32)
    beam.secret_key_to_public_key(res_sk, public_key)

    ## Signature_sign
    msg32 = public_key
    out_nonce_pub_x = bytearray(32)
    out_nonce_pub_y = bytearray(1)
    out_k = bytearray(32)
    beam.signature_sign(msg32, res_sk, out_nonce_pub_x, out_nonce_pub_y, out_k)

    text_to_send_back = 'Out nonce pub X:{}\nOut nonce pub Y:{}\nOut k:{}'.format(bin_to_str(out_nonce_pub_x),
                                                                                  bin_to_str(out_nonce_pub_y),
                                                                                  bin_to_str(out_k))
async def test_beam_signature_is_valid():
    ### TEST BEAM is_signature_valid
    # Get kdf
    mnemonic = storage.get_mnemonic()
    seed = beam.phrase_to_seed(mnemonic)
    seed_size = 32
    secret_key = bytearray(32)
    cofactor = bytearray(32)
    beam.seed_to_kdf(seed, seed_size, secret_key, cofactor)
    seed_str = ''.join('{:02x}'.format(x) for x in seed)

    # Generate hash id
    a_id = 123456
    a_type = 1113748301
    sub_idx = 0
    hash_id = bytearray(32)
    beam.generate_hash_id(a_id, a_type, sub_idx, hash_id)

    # Derive key
    res_sk = bytearray(32)
    beam.derive_child_key(secret_key, 32, hash_id, 32, cofactor, res_sk)

    # Secret key to public key
    public_key = bytearray(32)
    beam.secret_key_to_public_key(res_sk, public_key)

    # Signature_sign
    msg32 = public_key
    sign_nonce_pub_x = bytearray(32)
    sign_nonce_pub_y = bytearray(1)
    sign_k = bytearray(32)
    print('Message32: {}'.format(bin_to_str(msg32)))
    await require_confirm_sign_message(ctx, msg32)
    beam.signature_sign(msg32, res_sk, sign_nonce_pub_x, sign_nonce_pub_y, sign_k)

    # Is signature valid
    is_valid = beam.is_valid_signature(msg32, sign_nonce_pub_x, sign_nonce_pub_y, sign_k, res_sk);
    is_valid_msg = 'Sign_x: {}; Sign_y: {}; Sign_k: {}; Is valid: {}'.format(bin_to_str(sign_nonce_pub_x),
                                                                             bin_to_str(sign_nonce_pub_y),
                                                                             bin_to_str(sign_k),
                                                                             is_valid)
    await require_validate_sign_message(ctx, is_valid_msg)
    text_to_send_back = 'Is valid signature? : {}'.format(is_valid)
    # Modify to check the signature is no longer valid
    msg32[0] = msg32[0] + 1
    is_valid = beam.is_valid_signature(msg32, sign_nonce_pub_x, sign_nonce_pub_y, sign_k, res_sk);
    text_to_send_back += '\n(xfail) Is valid signature? : {}'.format(is_valid)

    return text_to_send_back

async def perform_test():
    text_to_send_back = test_beam_signature_is_valid()

    await beam_confirm_tx(ctx, 123456, 228)

    if msg.show_display:
        while True:
            #if await show_qr(ctx, text):
            #if await require_confirm(ctx, text, ButtonRequestType.ConfirmOutput):
            if await show_address(ctx, text_to_send_back, 'Hello from BEAM!'):
                print('BREAK!')
                break
            else:
                print("Processing..")

    print("Bye!")
    res = BeamConfirmResponseMessage(text=text_to_send_back, response=True)
    print(res)
    print(res.text)
    print(res.response)
    return res
