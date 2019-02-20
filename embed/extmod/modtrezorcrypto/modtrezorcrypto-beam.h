#include "py/objstr.h"

#include "beam.h"

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

STATIC const mp_rom_map_elem_t mod_trezorcrypto_beam_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_beam) },
    { MP_ROM_QSTR(MP_QSTR_hello_crypto_world), MP_ROM_PTR(&mod_trezorcrypto_beam_hello_crypto_world_obj) },
};

STATIC MP_DEFINE_CONST_DICT (
    mod_trezorcrypto_beam_globals,
    mod_trezorcrypto_beam_globals_table
);

STATIC const mp_obj_module_t mod_trezorcrypto_beam_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mod_trezorcrypto_beam_globals,
};
