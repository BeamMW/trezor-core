#include "py/objstr.h"

#include "beam/beam.h"
#include "beam/functions.h"

/// package: trezorcrypto.beam

/// def hello_crypto_world() -> int:
///     '''
///     Hello from BEAM's crypto world
///     '''
STATIC mp_obj_t mod_trezorcrypto_beam_hello_crypto_world(void) {
    int code = get_beam_hello();
    printf("Hello from Beam with code: %d", code);
    return mp_obj_new_int(code);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_trezorcrypto_beam_hello_crypto_world_obj, mod_trezorcrypto_beam_hello_crypto_world);

/// def seed(mnemonic: str, passphrase: str=None, callback: (int, int -> None)=None) -> bytes:
///     '''
///     Generate BEAM seed from mnemonic and passphrase.
///     '''
STATIC mp_obj_t mod_trezorcrypto_beam_phrase_to_seed(size_t n_args, const mp_obj_t* args) {
    mp_buffer_info_t mnemo;

    //TODO
    //mp_buffer_info_t phrase;

    mp_get_buffer_raise(args[0], &mnemo, MP_BUFFER_READ);
    uint8_t seed[32];
    const char* pmnemonic = mnemo.len > 0 ? mnemo.buf : "";
    //TODO
    //const char *ppassphrase = phrase.len > 0 ? phrase.buf : "";

    if (n_args > 2) {
        //TODO
        //// generate with a progress callback
        //ui_wait_callback = args[2];
        //beam_mnemonic_to_seed(pmnemonic, ppassphrase, seed, wrapped_ui_wait_callback);
        //ui_wait_callback = mp_const_none;
    } else {
        // generate without callback
        phrase_to_seed(pmnemonic, seed);
    }
    return mp_obj_new_bytes(seed, sizeof(seed));
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_trezorcrypto_beam_phrase_to_seed_obj, 1, 3, mod_trezorcrypto_beam_phrase_to_seed);

// void generate_hash_id(uint64_t idx, uint32_t type, uint32_t sub_idx, uint8_t *out32);

STATIC const mp_rom_map_elem_t mod_trezorcrypto_beam_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_beam) },
    { MP_ROM_QSTR(MP_QSTR_hello_crypto_world), MP_ROM_PTR(&mod_trezorcrypto_beam_hello_crypto_world_obj) },
    { MP_ROM_QSTR(MP_QSTR_phrase_to_seed), MP_ROM_PTR(&mod_trezorcrypto_beam_phrase_to_seed_obj) },
};

STATIC MP_DEFINE_CONST_DICT (
    mod_trezorcrypto_beam_globals,
    mod_trezorcrypto_beam_globals_table
);

STATIC const mp_obj_module_t mod_trezorcrypto_beam_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mod_trezorcrypto_beam_globals,
};
