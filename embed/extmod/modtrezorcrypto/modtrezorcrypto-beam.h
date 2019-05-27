#include "py/objstr.h"
#include "py/objint.h"

#include "beam/beam.h"
#include "beam/functions.h"
#include "beam/rangeproof.h"
#include "beam/kernel.h"
#include "beam/misc.h"


/// package: trezorcrypto.beam

//
#define DBG_PRINT(msg, arr, len) \
  printf(msg);                     \
  for (size_t i = 0; i < len; i++) \
  {                                \
    printf("%02x", ((int*)arr)[i]);        \
  }                                \
  printf("\n");

// To get_uint64_t and other helper functions
// TAKEN FROM: #include "modtrezorcrypto-monero.h"
static uint64_t mp_obj_uint64_get_checked_beam(mp_const_obj_t self_in) {
#if MICROPY_LONGINT_IMPL != MICROPY_LONGINT_IMPL_MPZ
#  error "MPZ supported only"
#endif

    if (MP_OBJ_IS_SMALL_INT(self_in)) {
        return MP_OBJ_SMALL_INT_VALUE(self_in);
    } else {
        byte buff[8];
        uint64_t res = 0;
        mp_obj_t * o = MP_OBJ_TO_PTR(self_in);

        mp_obj_int_to_bytes_impl(o, true, 8, buff);
        for (int i = 0; i<8; i++){
            res <<= i > 0 ? 8 : 0;
            res |= (uint64_t)(buff[i] & 0xff);
        }
        return res;
    }
}

static uint64_t mp_obj_get_uint64_beam(mp_const_obj_t arg) {
    if (arg == mp_const_false) {
        return 0;
    } else if (arg == mp_const_true) {
        return 1;
    } else if (MP_OBJ_IS_SMALL_INT(arg)) {
        return MP_OBJ_SMALL_INT_VALUE(arg);
    } else if (MP_OBJ_IS_TYPE(arg, &mp_type_int)) {
        return mp_obj_uint64_get_checked_beam(arg);
    } else {
        if (MICROPY_ERROR_REPORTING == MICROPY_ERROR_REPORTING_TERSE) {
            mp_raise_TypeError("can't convert to int");
        } else {
            nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_TypeError,
                                                    "can't convert %s to int", mp_obj_get_type_str(arg)));
        }
    }
}


typedef struct _mp_obj_key_idv_t {
    mp_obj_base_t base;
    key_idv_t kidv;
} mp_obj_key_idv_t;

STATIC const mp_obj_type_t mod_trezorcrypto_beam_key_idv_type;

typedef struct _mp_obj_beam_transaction_maker_t {
    mp_obj_base_t base;
    kidv_vec_t inputs;
    kidv_vec_t outputs;
} mp_obj_beam_transaction_maker_t;

STATIC const mp_obj_type_t mod_trezorcrypto_beam_transaction_maker_type;

//
// Constructors
//

/// class TransactionMaker:
///     '''
///     TransactionMaker serves as a facade to build and sign the transaction
///     '''
///
///     def __init__(self):
///         '''
///         Creates TransactionMaker object
///         '''
///
///     def add_input(self, input: KeyIDV):
///         '''
///         Adds input to the transaction
///         '''
///
///     def add_output(self, output: KeyIDV):
///         '''
///         Adds output to the transaction
///         '''
STATIC mp_obj_t mod_trezorcrypto_beam_transaction_maker_make_new(const mp_obj_type_t* type, size_t n_args, size_t n_kw, const mp_obj_t* args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    mp_obj_beam_transaction_maker_t* o = m_new_obj(mp_obj_beam_transaction_maker_t);
    o->base.type = type;

    vec_init(&o->inputs);
    vec_init(&o->outputs);

    return MP_OBJ_FROM_PTR(o);
}

STATIC mp_obj_t mod_trezorcrypto_beam_transaction_maker___del__(mp_obj_t self) {
    mp_obj_beam_transaction_maker_t* o = MP_OBJ_TO_PTR(self);

    //TODO: if we add support for nested kernels, we should also deinit all nested inputs/outputs list of these kernels
    // @see beam/misc.c in `transaction_free()` method
    //vec_deinit_inner_ptrs(&o->inputs, tx_input_t);
    //transaction_free_outputs(&o->outputs);

    vec_deinit(&o->inputs);
    vec_deinit(&o->outputs);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_trezorcrypto_beam_transaction_maker___del___obj, mod_trezorcrypto_beam_transaction_maker___del__);

STATIC mp_obj_t mod_trezorcrypto_beam_transaction_maker_add_input(mp_obj_t self, mp_obj_t kidv_input) {
    mp_obj_beam_transaction_maker_t* o = MP_OBJ_TO_PTR(self);
    mp_obj_key_idv_t* input_obj = MP_OBJ_TO_PTR(kidv_input);
    key_idv_t kidv;
    memcpy(&kidv, &input_obj->kidv, sizeof(key_idv_t));

    vec_push(&o->inputs, kidv);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_trezorcrypto_beam_transaction_maker_add_input_obj, mod_trezorcrypto_beam_transaction_maker_add_input);

STATIC mp_obj_t mod_trezorcrypto_beam_transaction_maker_add_output(mp_obj_t self, mp_obj_t kidv_output) {
    mp_obj_beam_transaction_maker_t* o = MP_OBJ_TO_PTR(self);
    mp_obj_key_idv_t* output_obj = MP_OBJ_TO_PTR(kidv_output);
    key_idv_t kidv;
    memcpy(&kidv, &output_obj->kidv, sizeof(key_idv_t));

    vec_push(&o->outputs, kidv);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_trezorcrypto_beam_transaction_maker_add_output_obj, mod_trezorcrypto_beam_transaction_maker_add_output);

/// class KeyIDV:
///     '''
///     Beam KeyIDV
///     '''
///
///     def __init__(self):
///         '''
///         Creates a KIDV object.
///         '''
///
///     def set(self, idx: uint, type: uint, sub_idx: uint, value: uint):
///         '''
///         Sets index, type, sub index and value of KIDV object.
///         '''
STATIC mp_obj_t mod_trezorcrypto_beam_key_idv_make_new(const mp_obj_type_t* type, size_t n_args, size_t n_kw, const mp_obj_t* args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    mp_obj_key_idv_t* o = m_new_obj(mp_obj_key_idv_t);
    o->base.type = type;

    key_idv_init(&o->kidv);

    return MP_OBJ_FROM_PTR(o);
}

STATIC mp_obj_t mod_trezorcrypto_beam_key_idv_set(size_t n_args, const mp_obj_t* args) {
    mp_obj_key_idv_t *o = MP_OBJ_TO_PTR(args[0]);

    uint64_t idx = mp_obj_get_uint64_beam(args[1]);
    uint32_t type = mp_obj_get_int(args[2]);
    uint32_t sub_idx = mp_obj_get_int(args[3]);
    uint64_t value = mp_obj_get_uint64_beam(args[4]);

    o->kidv.id.idx = idx;
    o->kidv.id.type = type;
    o->kidv.id.sub_idx = sub_idx;
    o->kidv.value = value;

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_trezorcrypto_beam_key_idv_set_obj, 5, 5, mod_trezorcrypto_beam_key_idv_set);

STATIC mp_obj_t mod_trezorcrypto_beam_key_idv___del__(mp_obj_t self) {
    mp_obj_key_idv_t *o = MP_OBJ_TO_PTR(self);
    memzero(&(o->kidv), sizeof(key_idv_t));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_trezorcrypto_beam_key_idv___del___obj, mod_trezorcrypto_beam_key_idv___del__);

///
///
///

static void gej_to_xy_bufs(secp256k1_gej* group_point, uint8_t* x_buf, uint8_t* y_buf) {
    point_t intermediate_point_t;
    int export_result = export_gej_to_point(group_point, &intermediate_point_t);
    if (export_result == 0)
        mp_raise_ValueError("Invalid data length (only 16, 20, 24, 28 and 32 bytes are allowed)");

    // Copy contents to out buffer
    memcpy(x_buf, intermediate_point_t.x, 32);
    //TODO: memset() instead?
    *y_buf = intermediate_point_t.y;
}

/// def hello_crypto_world() -> int:
///     '''
///     Hello from BEAM's crypto world
///     '''
STATIC mp_obj_t mod_trezorcrypto_beam_hello_crypto_world(void) {
    init_context();
    test_tx_kernel();
    free_context();

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

/// def generate_hash_id(idx: int, type: int, sub_idx: int, out32: bytes):
///     '''
///     Generate BEAM hash id.
///     '''
STATIC mp_obj_t mod_trezorcrypto_beam_generate_hash_id(size_t n_args, const mp_obj_t* args) {
    uint64_t idx = mp_obj_get_uint64_beam(args[0]);
    uint32_t type = mp_obj_get_int(args[1]);
    uint32_t sub_idx = mp_obj_get_int(args[2]);

    //uint8_t* out32 = (uint8_t*)arg_out32;
    mp_buffer_info_t out32;
    mp_get_buffer_raise(args[3], &out32, MP_BUFFER_RW);

    generate_hash_id(idx, type, sub_idx, (uint8_t*)out32.buf);
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_trezorcrypto_beam_generate_hash_id_obj, 4, 4, mod_trezorcrypto_beam_generate_hash_id);

/// def seed_to_kdf(seed: bytes, seed_size: int, out_gen32: bytes, out_cofactor: bytes):
///     '''
///     Transform seed to BEAM KDF
///     '''
STATIC mp_obj_t mod_trezorcrypto_beam_seed_to_kdf(size_t n_args, const mp_obj_t* args) {
    mp_buffer_info_t seed;
    mp_get_buffer_raise(args[0], &seed, MP_BUFFER_READ);

    uint64_t seed_size = mp_obj_get_uint64_beam(args[1]);

    mp_buffer_info_t out_gen32;
    mp_get_buffer_raise(args[2], &out_gen32, MP_BUFFER_RW);

    mp_buffer_info_t out_cofactor;
    mp_get_buffer_raise(args[3], &out_cofactor, MP_BUFFER_RW);

    scalar_t cofactor;
    //void seed_to_kdf(const uint8_t *seed, size_t seed_size, uint8_t *out_gen32, scalar_t *out_cof);
    seed_to_kdf((const uint8_t*)seed.buf, seed_size, (uint8_t*)out_gen32.buf, &cofactor);
    // Write data into out_cofactor raw pointer instead of scalar type
    scalar_get_b32((uint8_t*)out_cofactor.buf, &cofactor);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_trezorcrypto_beam_seed_to_kdf_obj, 4, 4, mod_trezorcrypto_beam_seed_to_kdf);

//void derive_key(const uint8_t *parent, uint8_t parent_size, const uint8_t *hash_id, uint8_t id_size, const scalar_t *cof_sk, scalar_t *out_res_sk);
STATIC mp_obj_t mod_trezorcrypto_beam_derive_child_key(size_t n_args, const mp_obj_t* args) {
    mp_buffer_info_t parent;
    mp_get_buffer_raise(args[0], &parent, MP_BUFFER_READ);

    uint8_t parent_size = mp_obj_get_int(args[1]);
    printf("parent size!: %d\n", parent_size);

    mp_buffer_info_t hash_id;
    mp_get_buffer_raise(args[2], &hash_id, MP_BUFFER_READ);

    uint8_t hash_id_size = mp_obj_get_int(args[3]);

    mp_buffer_info_t cofactor_sk;
    mp_get_buffer_raise(args[4], &cofactor_sk, MP_BUFFER_READ);
    scalar_t cof_sk;
    scalar_import_nnz(&cof_sk, (const uint8_t*)cofactor_sk.buf);

    mp_buffer_info_t out_res_sk;
    mp_get_buffer_raise(args[5], &out_res_sk, MP_BUFFER_RW);

    scalar_t res_sk;
    derive_key((const uint8_t*)parent.buf, parent_size, (const uint8_t*)hash_id.buf, hash_id_size, &cof_sk, &res_sk);

    // Write data into out_cofactor raw pointer instead of scalar type
    scalar_get_b32((uint8_t*)out_res_sk.buf, &res_sk);

    //DEBUG_PRINT("Got res: ", (uint8_t*)out_res_sk.buf, 32)

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_trezorcrypto_beam_derive_child_key_obj, 6, 6, mod_trezorcrypto_beam_derive_child_key);

STATIC mp_obj_t mod_trezorcrypto_beam_secret_key_to_public_key(mp_obj_t secret_key, mp_obj_t public_key_x, mp_obj_t public_key_y) {
    mp_buffer_info_t sk;
    mp_get_buffer_raise(secret_key, &sk, MP_BUFFER_READ);
    scalar_t scalar_sk;
    scalar_import_nnz(&scalar_sk, (const uint8_t*)sk.buf);

    mp_buffer_info_t pk_x;
    mp_get_buffer_raise(public_key_x, &pk_x, MP_BUFFER_RW);
    mp_buffer_info_t pk_y;
    mp_get_buffer_raise(public_key_y, &pk_y, MP_BUFFER_RW);

    //TODO<Kirill>: call it once on device initialization
    init_context();
    secp256k1_gej pk;
    generator_mul_scalar(&pk, get_context()->generator.G_pts, &scalar_sk);
    gej_to_xy_bufs(&pk, (uint8_t*)pk_x.buf, (uint8_t*)pk_y.buf);
    free_context();

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(mod_trezorcrypto_beam_secret_key_to_public_key_obj, mod_trezorcrypto_beam_secret_key_to_public_key);

STATIC mp_obj_t mod_trezorcrypto_beam_signature_sign(size_t n_args, const mp_obj_t* args) {
    mp_buffer_info_t msg32;
    mp_get_buffer_raise(args[0], &msg32, MP_BUFFER_READ);

    mp_buffer_info_t sk;
    mp_get_buffer_raise(args[1], &sk, MP_BUFFER_READ);
    scalar_t scalar_sk;
    scalar_import_nnz(&scalar_sk, (const uint8_t*)sk.buf);

    // in type of point_t_x (uint8_t[32])
    mp_buffer_info_t out_nonce_pub_x;
    mp_get_buffer_raise(args[2], &out_nonce_pub_x, MP_BUFFER_RW);

    // in type of point_t.y (uint8_t[1])
    mp_buffer_info_t out_nonce_pub_y;
    mp_get_buffer_raise(args[3], &out_nonce_pub_y, MP_BUFFER_RW);

    mp_buffer_info_t out_k;
    mp_get_buffer_raise(args[4], &out_k, MP_BUFFER_RW);

    //TODO<Kirill>: call it once on device initialization
    init_context();
    //void signature_sign(const uint8_t *msg32, const scalar_t *sk, const secp256k1_gej *generator_pts, secp256k1_gej *out_nonce_pub, scalar_t *out_k)
    ecc_signature_t signature;
    signature_sign((const uint8_t*)msg32.buf, &scalar_sk, get_context()->generator.G_pts, &signature);
    printf(" Internal! ---- out_nonce_pub  %d\n", (int)signature.nonce_pub.x.n[0]);
    // Export scalar
    // Write data into raw pointer instead of scalar type
    scalar_get_b32((uint8_t*)out_k.buf, &signature.k);
    gej_to_xy_bufs(&signature.nonce_pub, (uint8_t*)out_nonce_pub_x.buf, (uint8_t*)out_nonce_pub_y.buf);

    free_context();
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_trezorcrypto_beam_signature_sign_obj, 5, 5, mod_trezorcrypto_beam_signature_sign);

//int signature_is_valid(const uint8_t *msg32, const secp256k1_gej *nonce_pub, const scalar_t *k, const secp256k1_gej *pk, const secp256k1_gej *generator_pts);
STATIC mp_obj_t mod_trezorcrypto_beam_is_valid_signature(size_t n_args, const mp_obj_t* args) {
    mp_buffer_info_t msg32;
    mp_get_buffer_raise(args[0], &msg32, MP_BUFFER_READ);

    // Get nonce_pub
    // x part
    mp_buffer_info_t nonce_pub_x;
    mp_get_buffer_raise(args[1], &nonce_pub_x, MP_BUFFER_READ);
    // y part
    mp_buffer_info_t nonce_pub_y;
    mp_get_buffer_raise(args[2], &nonce_pub_y, MP_BUFFER_READ);
    // Convert nonce pub from two parts to point t
    point_t nonce_pub_point;
    memcpy(nonce_pub_point.x, nonce_pub_x.buf, 32);
    nonce_pub_point.y = *((int*)nonce_pub_y.buf);
    // Convert point t to secp256k1_gej nonce pub
    ecc_signature_t signature;
    point_import_nnz(&signature.nonce_pub, &nonce_pub_point);

    // Get scalar k
    mp_buffer_info_t k;
    mp_get_buffer_raise(args[3], &k, MP_BUFFER_READ);
    scalar_import_nnz(&signature.k, (const uint8_t*)k.buf);

    mp_buffer_info_t pk_x;
    mp_get_buffer_raise(args[4], &pk_x, MP_BUFFER_READ);
    mp_buffer_info_t pk_y;
    mp_get_buffer_raise(args[5], &pk_y, MP_BUFFER_READ);
    point_t pk_point;
    // Copy contents to out buffer
    memcpy(pk_point.x, pk_x.buf, 32);
    pk_point.y = *((int*)pk_y.buf);
    secp256k1_gej pk_gej;
    point_import_nnz(&pk_gej, &pk_point);

    init_context();
    int is_valid = signature_is_valid((const uint8_t*)msg32.buf, &signature, &pk_gej, get_context()->generator.G_pts);
    free_context();

    return mp_obj_new_int(is_valid);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_trezorcrypto_beam_is_valid_signature_obj, 6, 6, mod_trezorcrypto_beam_is_valid_signature);

STATIC mp_obj_t mod_trezorcrypto_beam_export_owner_key(size_t n_args, const mp_obj_t* args) {
    mp_buffer_info_t master_key32;
    mp_get_buffer_raise(args[0], &master_key32, MP_BUFFER_READ);

    mp_buffer_info_t master_cofactor;
    mp_get_buffer_raise(args[1], &master_cofactor, MP_BUFFER_READ);

    mp_buffer_info_t pin_code;
    mp_get_buffer_raise(args[2], &pin_code, MP_BUFFER_READ);
    size_t pin_size = mp_obj_get_int(args[3]);

    mp_buffer_info_t out_owner_key;
    mp_get_buffer_raise(args[4], &out_owner_key, MP_BUFFER_RW);

    scalar_t cofactor_scalar;
    scalar_import_nnz(&cofactor_scalar, (const uint8_t*)master_cofactor.buf);

    init_context();
    uint8_t* owner_key = get_owner_key((const uint8_t*)master_key32.buf, &cofactor_scalar, (const uint8_t*)pin_code.buf, pin_size);
    free_context();

    memcpy(out_owner_key.buf, owner_key, 145);
    free(owner_key);
    owner_key = NULL;

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_trezorcrypto_beam_export_owner_key_obj, 5, 5, mod_trezorcrypto_beam_export_owner_key);

STATIC mp_obj_t mod_trezorcrypto_beam_generate_key(size_t n_args, const mp_obj_t* args) {
    uint64_t idx = mp_obj_get_uint64_beam(args[0]);
    uint32_t type = mp_obj_get_int(args[1]);
    uint32_t sub_idx = mp_obj_get_int(args[2]);
    uint64_t value = mp_obj_get_uint64_beam(args[3]);

    uint32_t is_coin_key = mp_obj_get_int(args[4]);

    key_idv_t kidv;
    kidv.id.idx = idx;
    kidv.id.type = type;
    kidv.id.sub_idx = sub_idx;
    kidv.value = value;

    //TODO<Kirill>: call it once on device initialization
    init_context();

    mp_buffer_info_t seed;
    mp_get_buffer_raise(args[5], &seed, MP_BUFFER_READ);
    HKdf_t kdf;
    get_HKdf(0, (uint8_t*)seed.buf, &kdf);

    secp256k1_gej commitment;
    create_kidv_image(&kdf, &kidv, &commitment, is_coin_key);

    mp_buffer_info_t out_image_x;
    mp_get_buffer_raise(args[6], &out_image_x, MP_BUFFER_RW);

    mp_buffer_info_t out_image_y;
    mp_get_buffer_raise(args[7], &out_image_y, MP_BUFFER_RW);

    gej_to_xy_bufs(&commitment, (uint8_t*)out_image_x.buf, (uint8_t*)out_image_y.buf);

    free_context();

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_trezorcrypto_beam_generate_key_obj, 8, 8, mod_trezorcrypto_beam_generate_key);

STATIC mp_obj_t mod_trezorcrypto_beam_create_master_nonce(size_t n_args, const mp_obj_t* args) {
    mp_buffer_info_t master_nonce;
    mp_get_buffer_raise(args[0], &master_nonce, MP_BUFFER_RW);

    mp_buffer_info_t seed;
    mp_get_buffer_raise(args[1], &seed, MP_BUFFER_READ);

    create_master_nonce((uint8_t*)master_nonce.buf, (uint8_t*)seed.buf);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_trezorcrypto_beam_create_master_nonce_obj, 2, 2, mod_trezorcrypto_beam_create_master_nonce);

STATIC mp_obj_t mod_trezorcrypto_beam_create_derived_nonce(size_t n_args, const mp_obj_t* args) {
    mp_buffer_info_t master_nonce;
    mp_get_buffer_raise(args[0], &master_nonce, MP_BUFFER_READ);

    uint8_t idx = mp_obj_get_int(args[1]);

    mp_buffer_info_t out_new_nonce;
    mp_get_buffer_raise(args[2], &out_new_nonce, MP_BUFFER_RW);

    init_context();
    create_derived_nonce((const uint8_t*)master_nonce.buf, idx, (uint8_t*)out_new_nonce.buf);
    free_context();

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_trezorcrypto_beam_create_derived_nonce_obj, 3, 3, mod_trezorcrypto_beam_create_derived_nonce);

STATIC mp_obj_t mod_trezorcrypto_beam_get_nonce_public_key(
    size_t n_args, const mp_obj_t* args) {
    mp_buffer_info_t nonce;
    mp_get_buffer_raise(args[0], &nonce, MP_BUFFER_READ);

    // in type of point_t_x (uint8_t[32])
    mp_buffer_info_t out_nonce_pub_x;
    mp_get_buffer_raise(args[2], &out_nonce_pub_x, MP_BUFFER_RW);

    // in type of point_t.y (uint8_t[1])
    mp_buffer_info_t out_nonce_pub_y;
    mp_get_buffer_raise(args[3], &out_nonce_pub_y, MP_BUFFER_RW);

    init_context();
    point_t intermediate_point;
    get_nonce_public_key((const uint8_t*)nonce.buf, &intermediate_point);
    memcpy(out_nonce_pub_x.buf, intermediate_point.x, 32);
    memcpy(out_nonce_pub_y.buf, &intermediate_point.y, 1);
    free_context();

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_trezorcrypto_beam_get_nonce_public_key_obj, 3, 3, mod_trezorcrypto_beam_get_nonce_public_key);

STATIC mp_obj_t mod_trezorcrypto_beam_generate_rp_from_key_idv(size_t n_args, const mp_obj_t* args) {
    uint64_t idx = mp_obj_get_uint64_beam(args[0]);
    uint32_t type = mp_obj_get_int(args[1]);
    uint32_t sub_idx = mp_obj_get_int(args[2]);
    uint64_t value = mp_obj_get_uint64_beam(args[3]);

    key_idv_t kidv;
    kidv.id.idx = idx;
    kidv.id.type = type;
    kidv.id.sub_idx = sub_idx;
    kidv.value = value;

    mp_buffer_info_t asset_id;
    mp_get_buffer_raise(args[4], &asset_id, MP_BUFFER_READ);

    const uint8_t is_public = mp_obj_get_int(args[5]);

    mp_buffer_info_t seed;
    mp_get_buffer_raise(args[6], &seed, MP_BUFFER_READ);

    HKdf_t kdf;
    get_HKdf(0, (uint8_t*)seed.buf, &kdf);

    mp_buffer_info_t out_rp;
    mp_get_buffer_raise(args[7], &out_rp, MP_BUFFER_RW);

    init_context();
    rangeproof_create_from_key_idv(&kdf, (uint8_t*)out_rp.buf, &kidv, NULL, is_public);
    free_context();

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_trezorcrypto_beam_generate_rp_from_key_idv_obj, 8, 8, mod_trezorcrypto_beam_generate_rp_from_key_idv);

//
// Type defs
//

STATIC const mp_rom_map_elem_t mod_trezorcrypto_beam_key_idv_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&mod_trezorcrypto_beam_key_idv___del___obj) },
    { MP_ROM_QSTR(MP_QSTR_set), MP_ROM_PTR(&mod_trezorcrypto_beam_key_idv_set_obj) },
};
STATIC MP_DEFINE_CONST_DICT(mod_trezorcrypto_beam_key_idv_locals_dict, mod_trezorcrypto_beam_key_idv_locals_dict_table);

STATIC const mp_obj_type_t mod_trezorcrypto_beam_key_idv_type = {
    { &mp_type_type },
    .name = MP_QSTR_KeyIDV,
    .make_new = mod_trezorcrypto_beam_key_idv_make_new,
    .locals_dict = (void*)&mod_trezorcrypto_beam_key_idv_locals_dict,
};

STATIC const mp_rom_map_elem_t mod_trezorcrypto_beam_transaction_maker_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&mod_trezorcrypto_beam_key_idv___del___obj) },
    { MP_ROM_QSTR(MP_QSTR_add_input), MP_ROM_PTR(&mod_trezorcrypto_beam_transaction_maker_add_input_obj) },
    { MP_ROM_QSTR(MP_QSTR_add_output), MP_ROM_PTR(&mod_trezorcrypto_beam_transaction_maker_add_output_obj) },
};
STATIC MP_DEFINE_CONST_DICT(mod_trezorcrypto_beam_transaction_maker_locals_dict, mod_trezorcrypto_beam_transaction_maker_locals_dict_table);

STATIC const mp_obj_type_t mod_trezorcrypto_beam_transaction_maker_type = {
    { &mp_type_type },
    .name = MP_QSTR_TransactionMaker,
    .make_new = mod_trezorcrypto_beam_transaction_maker_make_new,
    .locals_dict = (void*)&mod_trezorcrypto_beam_transaction_maker_locals_dict,
};

STATIC const mp_rom_map_elem_t mod_trezorcrypto_beam_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_beam) },
    { MP_ROM_QSTR(MP_QSTR_hello_crypto_world), MP_ROM_PTR(&mod_trezorcrypto_beam_hello_crypto_world_obj) },
    { MP_ROM_QSTR(MP_QSTR_phrase_to_seed), MP_ROM_PTR(&mod_trezorcrypto_beam_phrase_to_seed_obj) },
    { MP_ROM_QSTR(MP_QSTR_generate_hash_id), MP_ROM_PTR(&mod_trezorcrypto_beam_generate_hash_id_obj) },
    { MP_ROM_QSTR(MP_QSTR_seed_to_kdf), MP_ROM_PTR(&mod_trezorcrypto_beam_seed_to_kdf_obj) },
    { MP_ROM_QSTR(MP_QSTR_derive_child_key), MP_ROM_PTR(&mod_trezorcrypto_beam_derive_child_key_obj) },
    { MP_ROM_QSTR(MP_QSTR_secret_key_to_public_key), MP_ROM_PTR(&mod_trezorcrypto_beam_secret_key_to_public_key_obj) },
    { MP_ROM_QSTR(MP_QSTR_signature_sign), MP_ROM_PTR(&mod_trezorcrypto_beam_signature_sign_obj) },
    { MP_ROM_QSTR(MP_QSTR_is_valid_signature), MP_ROM_PTR(&mod_trezorcrypto_beam_is_valid_signature_obj) },
    { MP_ROM_QSTR(MP_QSTR_export_owner_key), MP_ROM_PTR(&mod_trezorcrypto_beam_export_owner_key_obj) },
    { MP_ROM_QSTR(MP_QSTR_generate_key), MP_ROM_PTR(&mod_trezorcrypto_beam_generate_key_obj) },
    { MP_ROM_QSTR(MP_QSTR_create_master_nonce), MP_ROM_PTR(&mod_trezorcrypto_beam_create_master_nonce_obj) },
    { MP_ROM_QSTR(MP_QSTR_create_derived_nonce), MP_ROM_PTR(&mod_trezorcrypto_beam_create_derived_nonce_obj) },
    { MP_ROM_QSTR(MP_QSTR_generate_rp_from_key_idv), MP_ROM_PTR(&mod_trezorcrypto_beam_generate_rp_from_key_idv_obj) },
    { MP_ROM_QSTR(MP_QSTR_KeyIDV), MP_ROM_PTR(&mod_trezorcrypto_beam_key_idv_type) },
    { MP_ROM_QSTR(MP_QSTR_TransactionMaker), MP_ROM_PTR(&mod_trezorcrypto_beam_transaction_maker_type) },
};

STATIC MP_DEFINE_CONST_DICT (
    mod_trezorcrypto_beam_globals,
    mod_trezorcrypto_beam_globals_table
);

STATIC const mp_obj_module_t mod_trezorcrypto_beam_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mod_trezorcrypto_beam_globals,
};
