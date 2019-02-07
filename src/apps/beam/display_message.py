from trezor import ui
from trezor.messages import ButtonRequestType
from trezor.ui.text import Text

#from apps.common.layout import show_qr
from apps.common.confirm import *

async def display_message(ctx, msg):
    text = Text(msg.text, ui.ICON_SEND, icon_color=ui.GREEN)
    text.bold("BEAM")
    print("Received new message call:")
    print(msg.show_display)
    if msg.show_display:
        while True:
            #if await show_qr(ctx, text):
            if await require_confirm(ctx, text, ButtonRequestType.SignTx):
                break
