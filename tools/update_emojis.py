#!/bin/python

# SPDX-FileCopyrightText: 2026 Siddharth Chopra <contact.sid.chopra@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause

import json
import requests

# run this file from src/emojis directory - python3 ./tools/update_emojis.py

# supported locales listed at https://app.unpkg.com/emojibase-data@17.0.0
# ignoring es-mx and en-gb
locales = [
    "bn",
    "da",
    "de",
    "en",
    "es",
    "et",
    "fi",
    "fr",
    "hi",
    "hu",
    "it",
    "ja",
    "ko",
    "lt",
    "ms",
    "nb",
    "nl",
    "pl",
    "pt",
    "ru",
    "sv",
    "th",
    "uk",
    "vi",
    "zh"
]

with open("emoji_shortnames.h", "w") as f:
    f.write(f'''// SPDX-FileCopyrightText: None
// SPDX-License-Identifier: LGPL-2.0-or-later
// THIS FILE WAS AUTO-GENERATED
// clang-format off
#include <iterator>

struct Emoji {{
    const char* shortname;
    const char* code;
}};

''')
    
for locale in locales:
    url_normal = f"https://unpkg.com/emojibase-data@17.0.0/{locale}/shortcodes/cldr.json"
    url_native = f"https://unpkg.com/emojibase-data@17.0.0/{locale}/shortcodes/cldr-native.json"
    res_normal = requests.get(url_normal)
    res_native = requests.get(url_native)
    
    if (locale == 'ms'): # has no native json
        pass
    elif (res_normal.status_code != 200 or res_native.status_code != 200):
        raise ConnectionError(f"FAILED TO FETCH EMOJIS FOR LOCALE {locale}")
        
    j_normal = res_normal.json()
    j_normal_rev = dict(zip(j_normal.values(), j_normal.keys()))

    j_native = res_native.json() if locale != 'ms' else {}
    j_native_rev = dict(zip(j_native.values(), j_native.keys()))

    merged = j_normal_rev | j_native_rev
    
    with open(f'shortnames_{locale}.h', 'w') as f:
        f.write('''// SPDX-FileCopyrightText: None
// SPDX-License-Identifier: LGPL-2.0-or-later
// THIS FILE WAS AUTO-GENERATED
// clang-format off
''')
        f.write(f'''inline constexpr Emoji emojis_{locale}[] = {{
''')

        for key in merged:
            f.write(f'''    {{"{key}", "{merged[key]}"}},\n''')

        f.write('''};\n\n''')
        
    with open('emoji_shortnames.h', 'a') as f:
        f.write(f'#include "shortnames_{locale}.h"\n')
        
        
with open('emoji_shortnames.h', 'a') as f:
    f.write('''\nstruct Locale {
    const char* locale;
    const Emoji* emoji_arr;
    const size_t emoji_arr_size;
};
            
inline constexpr Locale supported_locales[] = {
''')
    
    for locale in locales:
        f.write(f'''    {{"{locale}", emojis_{locale}, std::size(emojis_{locale})}},
''')
        
    f.write('};')
