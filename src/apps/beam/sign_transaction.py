from trezor.crypto import beam

from trezor.messages.BeamSignTransaction import BeamSignTransaction
from trezor.messages.BeamSignedTransaction import BeamSignedTransaction

from apps.common import storage
from apps.beam.layout import *
from apps.beam.nonce import consume_nonce

async def sign_transaction(ctx, msg):
    tm = beam.TransactionMaker()
    tm.set_transaction_data(
        msg.kernel_params.fee,
        msg.kernel_params.min_height, msg.kernel_params.max_height,
        msg.kernel_params.commitment.x, msg.kernel_params.commitment.y,
        msg.kernel_params.multisig_nonce.x, msg.kernel_params.multisig_nonce.y,
        msg.nonce_slot,
        msg.offset_sk)

    #print("Inputs:")
    for input in msg.inputs:
        kidv = beam.KeyIDV()
        kidv.set(input.idx, input.type, input.sub_idx, input.value)
        tm.add_input(kidv)

    #print("Outputs:")
    for output in msg.outputs:
        kidv = beam.KeyIDV()
        kidv.set(output.idx, output.type, output.sub_idx, output.value)
        tm.add_output(kidv)

    mnemonic = storage.get_mnemonic()
    seed = beam.from_mnemonic_beam(mnemonic)
    sk_total = bytearray(32)

    value_transferred = tm.sign_transaction_part_1(seed, sk_total)

    tx_action_message = 'RECEIVE' if value_transferred <= 0 else 'TRANSFER'
    await beam_confirm_message(ctx, tx_action_message + ': ', str(abs(value_transferred)), False)

    signature = bytearray(32)
    nonce = consume_nonce(msg.nonce_slot)
    is_signed = tm.sign_transaction_part_2(sk_total, nonce, signature)

    return BeamSignedTransaction(signature=signature)

