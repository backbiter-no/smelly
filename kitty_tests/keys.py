#!/usr/bin/env python3
# vim:fileencoding=utf-8
# License: GPL v3 Copyright: 2016, Kovid Goyal <kovid at kovidgoyal.net>

from functools import partial
import kitty.fast_data_types as defines
from . import BaseTest


class TestKeys(BaseTest):

    def test_encode_key_event(self):
        enc = defines.encode_key_for_tty
        ae = self.assertEqual
        shift, alt, ctrl, super = defines.GLFW_MOD_SHIFT, defines.GLFW_MOD_ALT, defines.GLFW_MOD_CONTROL, defines.GLFW_MOD_SUPER  # noqa
        press, repeat, release = defines.GLFW_PRESS, defines.GLFW_REPEAT, defines.GLFW_RELEASE  # noqa

        def csi(mods=0, num=1, trailer='u'):
            ans = f'\033[{num}'
            if mods:
                m = 0
                if mods & shift:
                    m |= 1
                if mods & alt:
                    m |= 2
                if mods & ctrl:
                    m |= 4
                if mods & super:
                    m |= 8
                ans += f';{m+1}'
            return ans + trailer

        def mods_test(key, plain, shift=None, ctrl=None, alt=None, calt=None, cshift=None, ashift=None, csi_num=None, trailer='u'):
            c = partial(csi, num=csi_num or key, trailer=trailer)
            e = partial(enc, key=key)
            ae(e(), plain)
            ae(e(mods=defines.GLFW_MOD_SHIFT), shift or c(defines.GLFW_MOD_SHIFT))
            ae(e(mods=defines.GLFW_MOD_CONTROL), ctrl or c(defines.GLFW_MOD_CONTROL))
            ae(e(mods=defines.GLFW_MOD_ALT | defines.GLFW_MOD_CONTROL), calt or c(defines.GLFW_MOD_ALT | defines.GLFW_MOD_CONTROL))
            ae(e(mods=defines.GLFW_MOD_SHIFT | defines.GLFW_MOD_CONTROL), cshift or c(defines.GLFW_MOD_CONTROL | defines.GLFW_MOD_SHIFT))
            ae(e(mods=defines.GLFW_MOD_SHIFT | defines.GLFW_MOD_ALT), ashift or c(defines.GLFW_MOD_ALT | defines.GLFW_MOD_SHIFT))

        mods_test(defines.GLFW_FKEY_ENTER, '\x0d', alt='\033\x0d', csi_num=ord('\r'))
        mods_test(defines.GLFW_FKEY_ESCAPE, '\x1b', alt='\033\033', csi_num=27)
        mods_test(defines.GLFW_FKEY_BACKSPACE, '\x7f', alt='\033\x7f', csi_num=127)
        mods_test(defines.GLFW_FKEY_TAB, '\t', alt='\033\t', shift='\x1b[Z', csi_num=ord('\t'))

        q = partial(enc, key=ord('a'))
        ae(q(), 'a')
        ae(q(text='a'), 'a')
        ae(q(action=repeat), 'a')
        ae(q(action=release), '')

    def test_encode_mouse_event(self):
        NORMAL_PROTOCOL, UTF8_PROTOCOL, SGR_PROTOCOL, URXVT_PROTOCOL = range(4)
        L, M, R = 1, 2, 3
        protocol = SGR_PROTOCOL

        def enc(button=L, action=defines.PRESS, mods=0, x=1, y=1):
            return defines.test_encode_mouse(x, y, protocol, button, action, mods)

        self.ae(enc(), '<0;1;1M')
        self.ae(enc(action=defines.RELEASE), '<0;1;1m')
        self.ae(enc(action=defines.MOVE), '<35;1;1M')
        self.ae(enc(action=defines.DRAG), '<32;1;1M')

        self.ae(enc(R), '<2;1;1M')
        self.ae(enc(R, action=defines.RELEASE), '<2;1;1m')
        self.ae(enc(R, action=defines.DRAG), '<34;1;1M')

        self.ae(enc(M), '<1;1;1M')
        self.ae(enc(M, action=defines.RELEASE), '<1;1;1m')
        self.ae(enc(M, action=defines.DRAG), '<33;1;1M')

        self.ae(enc(x=1234, y=5678), '<0;1234;5678M')
        self.ae(enc(mods=defines.GLFW_MOD_SHIFT), '<4;1;1M')
        self.ae(enc(mods=defines.GLFW_MOD_ALT), '<8;1;1M')
        self.ae(enc(mods=defines.GLFW_MOD_CONTROL), '<16;1;1M')
