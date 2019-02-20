from trezor import ui
from trezor.crypto import beam
from trezor.messages import ButtonRequestType
from trezor.messages.BeamConfirmResponseMessage import BeamConfirmResponseMessage
from trezor.ui.text import Text

#from apps.common.layout import show_qr
from apps.common.confirm import *
from apps.common.layout import *

async def display_message(ctx, msg):
    text = Text(msg.text, ui.ICON_SEND, icon_color=ui.GREEN)
    text.bold("BEAM")
    print("Received new message call:")
    print(msg.show_display)
    if msg.show_display:
        while True:
            #if await show_qr(ctx, text):
            #if await require_confirm(ctx, text, ButtonRequestType.ConfirmOutput):
            if await show_address(ctx, 'some_addr', 'description!!!!'):
                print('BREAK!')
                #break
            else:
                print("Processing..")
    text_to_send_back = msg.text + ' This is a message from device! Received code:' + str(beam.hello_crypto_world())
    print("Bye!")
    res = BeamConfirmResponseMessage(text=text_to_send_back, response=True)
    print(res)
    print(res.text)
    print(res.response)
    return res
