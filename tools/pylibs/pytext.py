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
# Desc      It represents a text manipulation functions.
# ---------------------------------------------------------------------------- #

import re
import pylog

def wrap(
        text,
        width=80,
        first_indent=0,
        next_indent=0,
        html_br_only=False
        ) -> str:
    """This function takes text and wrap it around a certain width.

    [ Rules of wrapping ]

    § The text lines breaks can be determine based on normal '\\n' character\
    or by a specified line break tag similar to the HTML tags <br>.

    § The wrapped text can be indented by two parameter one determine the\
    indentation of the first line and the other determine the indentation of
    the next lines.

    § All adjacent white-spaces are considered one space

    § some other formatting tags are accepted such as:
        * </no-space> all previous and after white spaces are cancelled.
        * </tab>      for tab space insertion with four spaces.
        * </indent>   all next line to this tag will be indented by four spaces
        * </dndent>   all next line to this tag will be dendented by four spaces
    """

    if html_br_only:
        text_lines = text.strip().split('<br>')
    else:
        text_lines = text.strip().splitlines()

    lines = []
    first_line = True
    delta_indent = 0
    for line in text_lines:

        line = re.sub(r'\s+', ' ', line).strip()
        line = re.sub(r'\s*<\/no-space>\s*', '', line)
        line = re.sub(r'\s*<\/tab>\s*', ' </tab> ', line)
        line = re.sub(r'\s*<\/indent>\s*', ' </indent> ', line)
        line = re.sub(r'\s*<\/dndent>\s*', ' </dndent> ', line)

        if line == '':
            lines.append('')

        line_str = ''
        line_len = 0
        first_word = True
        def init_line():
            nonlocal first_word
            nonlocal first_line
            nonlocal line_str
            nonlocal line_len
            first_word = True
            line_len = delta_indent
            if first_line:
                first_line = False
                line_str = ' ' * first_indent
            else:
                line_str = ' ' * (next_indent + delta_indent)

        init_line()

        for word in line.split():
            tab = False
            if word == '</tab>':
                tab = True
                first_word = True
                word = '   '
                pass
            elif word == '</indent>':
                delta_indent += 4
                continue
            elif word == '</dndent>':
                delta_indent -= 4
                continue
            w_len = pylog.get_colored_str_len(word)
            if w_len == 0:
                line_str += word
                continue
            word_len = w_len + (1 if not first_word else 0)

            if line_len + word_len <= width:
                if first_word:
                    first_word = False
                    line_str += word
                    line_len += word_len
                else:
                    line_str += ' ' + word
                    line_len += word_len
            else:
                lines.append(line_str)
                init_line()
                if not tab:
                    line_str += word
                    line_len += word_len
                    first_word = False

        if not first_word:
            lines.append(line_str)

    return '\n'.join(lines)

# --- end of file ------------------------------------------------------------ #
