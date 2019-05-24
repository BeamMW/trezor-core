from trezor.crypto import beam
from trezorcrypto import sha1

from trezor.messages.BeamSignTransaction import BeamSignTransaction
from trezor.messages.BeamSignedTransaction import BeamSignedTransaction

async def sign_transaction(ctx, msg):
    print(dir(beam))
    print(dir(sha1))

    tm = beam.TransactionMaker()
    kidv = beam.KeyIDV()
    kidv.set(0, 1, 2, 3)
    tm.add_input(kidv)
    print(kidv)

    return BeamSignedTransaction(signature=bytearray(10))


class Transaction:
    def __init__(
        self,
        inputs: list,
        outputs: list,
        transactions: list,
        keychain,
        protocol_magic: int,
    ):
        pass
