from common import *

from apps.beam.get_public_key import get_public_key
from trezor.messages.BeamECCPoint import BeamECCPoint

from apps.beam.helpers import (
    bin_to_str,
    get_beam_kdf,
    get_beam_pk,
)

class TestBeamGetPublicKey(unittest.TestCase):
    def test_get_public_key_1(self):
        mnemonic = "all all all all all all all all all all all all"
        kdf = get_beam_kdf(mnemonic)

        pk = get_beam_pk(0, 0, kdf)
        self.assertEqual("88b528eecb5ee5ae81e56e2105aca06997761c9cd2e566b25eaee1951be26688", bin_to_str(pk[0]))
        self.assertEqual(1, pk[1])

    def test_get_public_key_2(self):
        mnemonic = "abc abc abc abc abc abc abc abc abc abc abc abc"
        kdf = get_beam_kdf(mnemonic)

        pk = get_beam_pk(0, 0, kdf)
        self.assertEqual("119a034ddd950028e45ee1d0c9b26efba0fb28af83e7f2ba83a5c0fa7ae2daee", bin_to_str(pk[0]))
        self.assertEqual(0, pk[1])

    def test_get_public_key_another_sub_idx(self):
        mnemonic = "abc abc abc abc abc abc abc abc abc abc abc abc"
        kdf = get_beam_kdf(mnemonic)

        pk = get_beam_pk(0, 1, kdf)
        self.assertEqual("2a4c57b74f1cc0ee995ceab14ae9e9bc581da8ccb290564d15b9079858635604", bin_to_str(pk[0]))
        self.assertEqual(1, pk[1])


if __name__ == '__main__':
    unittest.main()

