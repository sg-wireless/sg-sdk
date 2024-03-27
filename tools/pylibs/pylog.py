# ---------------------------------------------------------------------------- #
# Copyright (c) 2023-2024 SG Wireless - All Rights Reserved
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files(the “Software”), to deal
# in the Software without restriction, including without limitation the rights
# to use,  copy,  modify,  merge, publish, distribute, sublicense, and/or sell
# copies  of  the  Software,  and  to  permit  persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED “AS IS”,  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
# IMPLIED,  INCLUDING BUT NOT LIMITED TO  THE  WARRANTIES  OF  MERCHANTABILITY
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# AUTHORS  OR  COPYRIGHT  HOLDERS  BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN  CONNECTION WITH  THE SOFTWARE OR  THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# Author    Ahmed Sabry (SG Wireless)
#
# Desc      This is a python logging library
# ---------------------------------------------------------------------------- #

# ---------------------------------------------------------------------------- #
# imports
# ---------------------------------------------------------------------------- #

import re
import os

# ---------------------------------------------------------------------------- #
# colors and effects constants
# ---------------------------------------------------------------------------- #

BLACK       = 30
RED         = 31
GREEN       = 32
YELLOW      = 33
BLUE        = 34
PURPLE      = 35
CYAN        = 36
WHITE       = 37
DEFAULT     = 39
GREY        = 90

CLEAR       = 0
HILIGHT     = 0
BOLD        = 1
DIMMED      = 2
ITALIC      = 3
UNDERLINE   = 4

LEFT        = 0
CENTER      = 1
RIGHT       = 2

disable_colors = os.environ.get('DISABLE_COLORS', 'False') == 'True'

# ---------------------------------------------------------------------------- #
# basic log class
# ---------------------------------------------------------------------------- #

class PyLog:
    _tab = ''
    _tab_indent = 0
    _line_started = False
    __bkg_color = False
    __bkg_color_str = ''

    def print(self, msg, color=DEFAULT, endl=True):
        if type(msg) != str:
            msg = f'{msg}' 
        if not self._line_started:
            print(self._tab, end='')
            self._line_started = True
        if False and self.__bkg_color and not disable_colors:
            # msg = clear_colors(msg)
            print(msg, end='\n' if endl else '')
        else:
            if disable_colors:
                msg = clear_colors(msg)
            print(self.__bkg_color_str + self.get_color_str(color) + msg + 
                self.get_color_str(DEFAULT),
                end='\n' if endl else '')
        if endl:
            self._line_started = False

    def flush(self):
        print('', end='', flush=True)
    
    def bkg_color(self, color=DEFAULT):
        if disable_colors:
            return
        __bkg_color_map = {
            BLACK   : 40,
            RED     : 41,
            GREEN   : 42,
            YELLOW  : 43,
            BLUE    : 44,
            PURPLE  : 45,
            CYAN    : 46,
            WHITE   : 107,
            DEFAULT : ''
        }
        self.__bkg_color = not (color == DEFAULT)
        self.__bkg_color_str = '\x1b[{}m'.format(__bkg_color_map[color])
        print(self.__bkg_color_str, end='')

    def get_color_str(self, color):
        if disable_colors:
            return ''
        if color == 0:
            color = ''
        return '\033[' + str(color) + 'm'

    def tab_inc(self):
        self._tab_indent += 1
        self._tab = '    ' * self._tab_indent

    def tab_dec(self):
        if self._tab_indent > 0:
            self._tab_indent -= 1
            self._tab = '    ' * self._tab_indent
    
    def tab_size(self) -> int:
        return len(self._tab)

    def log_line(self, length=80, char='=', color=DEFAULT):
        self.print(char * length, color)

# standard output object
__stdout = PyLog()

# ---------------------------------------------------------------------------- #
# module methods
# ---------------------------------------------------------------------------- #

def clear_colors(text: str) ->str:
    """Removes the colr terminal information from a string
    Parameters:
        text(str) : input text that contain the terminal color info

    Returns:
        a new string free of terminal color info
    """
    return re.sub(r'\x1b\[\d*m', '', text)

def log_disable_colors():
    global disable_colors
    disable_colors = True

def log(msg, color=DEFAULT, endl=True):
    __stdout.print(msg, color, endl)

def log_bkg_color(color):
    __stdout.bkg_color(color)

def log_flush():
    __stdout.flush()

def log_line(length=80, char='=', color=DEFAULT):
    __stdout.log_line(length, char, color)

def log_field(msg, width, align=LEFT, color=DEFAULT, endl=False,
              fill=' ', fill_color=DEFAULT):
    msg_len = get_colored_str_len(msg)
    if msg_len >= width:
        log(msg, color, endl)
        return

    pad_len = width - msg_len
    if align == CENTER:
        left_pad = pad_len // 2
    elif align == RIGHT:
        left_pad = pad_len
    else:
        left_pad = 0
    
    if left_pad:
        pad_len -= left_pad
        pad_str = ''
        if left_pad > 1:
            pad_str += fill * (left_pad - 1)
        pad_str += ' '
        log(pad_str, color=fill_color, endl=False)

    log(msg, color, endl=False)

    if pad_len:
        pad_str = ' '
        if pad_len > 1:
            pad_str += fill * (pad_len - 1)
        log(pad_str, color=fill_color, endl=endl)
    else:
        log('', endl=endl)


def log_tab_inc(tabs=1):
    while tabs:
        __stdout.tab_inc()
        tabs -= 1


def log_tab_dec(tabs=1):
    while tabs:
        __stdout.tab_dec()
        tabs -= 1

def log_tab_size():
    return __stdout.tab_size()

def log_get_color_str(color):
    return __stdout.get_color_str(color)

COLOR_DEFAULT = log_get_color_str(DEFAULT)
COLOR_GREY    = log_get_color_str(GREY)
COLOR_RED     = log_get_color_str(RED)
COLOR_YELLOW  = log_get_color_str(YELLOW)
COLOR_GREEN   = log_get_color_str(GREEN)
COLOR_BLUE    = log_get_color_str(BLUE)
COLOR_PURPLE  = log_get_color_str(PURPLE)
COLOR_CYAN    = log_get_color_str(CYAN)

COLOR_DIMMED    = log_get_color_str(DIMMED)
COLOR_HILIGHT   = log_get_color_str(HILIGHT)

FORMAT_CLEAR        = log_get_color_str(CLEAR)
FORMAT_BOLD         = log_get_color_str(BOLD)
FORMAT_ITALIC       = log_get_color_str(ITALIC)
FORMAT_UNDERLINED   = log_get_color_str(UNDERLINE)

__default__ = log_get_color_str(DEFAULT)
__grey__    = log_get_color_str(GREY)
__red__     = log_get_color_str(RED)
__yellow__  = log_get_color_str(YELLOW)
__green__   = log_get_color_str(GREEN)
__blue__    = log_get_color_str(BLUE)
__purple__  = log_get_color_str(PURPLE)
__cyan__    = log_get_color_str(CYAN)

__dimmed__    = log_get_color_str(DIMMED)
__hilight__    = log_get_color_str(HILIGHT)

__reset__   = log_get_color_str(CLEAR)
__bold__    = log_get_color_str(BOLD)
__italic__  = log_get_color_str(ITALIC)
__underline__  = log_get_color_str(UNDERLINE)



def log_decorated_header(caption, width=80, fill_char='-',
                         caption_color=DEFAULT):
    # -- [ <caption> ] --------
    left = 0
    rem_len = width - len(caption) - 2 - left
    s = log_get_color_str(WHITE) + log_get_color_str(DIMMED) \
        + fill_char * left + '(' \
        + log_get_color_str(HILIGHT) + log_get_color_str(caption_color) \
        + caption + log_get_color_str(WHITE) + log_get_color_str(DIMMED) \
        + ')' + fill_char * rem_len + log_get_color_str(HILIGHT)
    log(s)

def log_underlined(caption, caption_color=DEFAULT, underline_char='-'):
    length = len(caption)
    log(caption, color=caption_color)
    log_line(length=length, char=underline_char, color=GREY)

def get_colored_str_len(str):
    ll = len(str)
    for m in re.findall(r'\033\[\d*m', str):
        ll -= len(m)
    return ll

def get_color_len(str, x) -> int:
    ll = len(str)
    if str[x] == '\033' and x+1 < ll and str[x+1] == '[':
        if x+2 < ll:
            if str[x+2] == 'm':
                return 3
            elif str[x+2] == '3' and x+3 < ll and \
                  str[x+3] > '0' and str[x+3] <= '9':
                return 5
    return 0

def get_colored_str_pos(str, idx) -> int:
    ll = len(str)

    j = 0
    i = 0

    while i < ll:
        if j == idx:
            return i

        color_len = get_color_len(str, i)
        if color_len:
            i += color_len
        else:
            i += 1
            j += 1

    return i

def get_last_used_color_str(str) -> str:
    if disable_colors:
        return ''
    ll = len(str)
    i = 0
    color_str = log_get_color_str(DEFAULT)
    while i < ll:
        color_len = get_color_len(str, i)
        if color_len:
            color_str = str[i : i + color_len]
            i += color_len
        else:
            i += 1

    return color_str

def log_list(list, horiz=False, wrap_after=0, color=DEFAULT,
             first_indent=None,
             indent=4, numbering=False, numbering_color=DEFAULT,
             item_width=None, remove_prefix='', total_width=0):
    max_item_len = 0
    if item_width:
        max_item_len = item_width
    else:
        for i in list:
            ll = get_colored_str_len(i.removeprefix(remove_prefix))
            if ll > max_item_len:
                max_item_len = ll
        max_item_len += 3

    items_count = len(list)

    if total_width != 0 and wrap_after == 0:
        wrap_after = total_width // max_item_len

    if wrap_after == 0:
        rows = 1
        cols = items_count
    else:
        rows = (items_count // wrap_after)
        cols = wrap_after
        rows += 1 if rows * cols < items_count else 0
    if not horiz:
        rows, cols = cols, rows

    numbering_field_w = 1
    numbering_counter = 1
    if numbering:
        limit = 10
        while items_count >= limit:
            numbering_field_w += 1
            limit *= 10

    for row in range(0, rows):
        if row == 0 and type(first_indent) == int:
            log(' ' * first_indent, endl=False)
        else:
            log(' ' * indent, endl=False)
        for col in range(0, cols):
            idx = row * cols + col
            if idx >= items_count:
                break
            item = list[row * cols + col].removeprefix(remove_prefix)
            ll = get_colored_str_len(item)
            rem = max_item_len - ll
            if numbering:
                num_str = '[{:{}}] '.format(
                    numbering_counter, numbering_field_w)
                log(num_str, endl=False, color=numbering_color)
                numbering_counter += 1
                rem -= len(num_str)
            log(item, color=color, endl=False)
            log(' ' * rem, endl=False)
        log('')

def log_obj(obj: object, endl=True):
    if type(obj) == list:
        if len(obj) == 0:
            log('[]', endl=endl)
            return
        log('[')
        log_tab_inc()
        first = True
        for li in obj:
            if not first:
                log(',')
            first = False
            log_obj(li, False)
        log_tab_dec()
        log('')
        log(']', endl=endl)
        pass
    elif type(obj) == dict:
        log('{')
        log_tab_inc()
        first = True
        for k in obj:
            if not first:
                log(',')
            first = False
            log(f'{__hilight__}{__blue__}{k}{__reset__}: ', endl=False)
            log_obj(obj[k], endl=False)
        log_tab_dec()
        log('')
        log('}', endl=endl)
        pass
    else:
        if type(obj) == str:
            obj = f"'{__hilight__}{__red__}{obj}{__reset__}'"
        elif type(obj) in [int, float]:
            obj = f"{__hilight__}{__cyan__}{obj}{__reset__}"
        elif type(obj) == bool:
            obj = f"{__green__}{obj}{__reset__}"
        elif obj == None:
            obj = f"{__purple__}{obj}{__reset__}"

        log(obj, endl=endl)
    pass

# --- end of file  ----------------------------------------------------------- #
