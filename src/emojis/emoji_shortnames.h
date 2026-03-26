// THIS FILE WAS AUTO-GENERATED
// clang-format off
#include <iterator>

struct Emoji {
    const char* shortname;
    const char* code;
};

#include "shortnames_bn.h"
#include "shortnames_da.h"
#include "shortnames_de.h"
#include "shortnames_en.h"
#include "shortnames_es.h"
#include "shortnames_et.h"
#include "shortnames_fi.h"
#include "shortnames_fr.h"
#include "shortnames_hi.h"
#include "shortnames_hu.h"
#include "shortnames_it.h"
#include "shortnames_ja.h"
#include "shortnames_ko.h"
#include "shortnames_lt.h"
#include "shortnames_ms.h"
#include "shortnames_nb.h"
#include "shortnames_nl.h"
#include "shortnames_pl.h"
#include "shortnames_pt.h"
#include "shortnames_ru.h"
#include "shortnames_sv.h"
#include "shortnames_th.h"
#include "shortnames_uk.h"
#include "shortnames_vi.h"
#include "shortnames_zh.h"

struct Locale {
    const char* locale;
    const Emoji* emoji_arr;
    const size_t emoji_arr_size;
};
            
inline constexpr Locale supported_locales[] = {
    {"bn", emojis_bn, std::size(emojis_bn)},
    {"da", emojis_da, std::size(emojis_da)},
    {"de", emojis_de, std::size(emojis_de)},
    {"en", emojis_en, std::size(emojis_en)},
    {"es", emojis_es, std::size(emojis_es)},
    {"et", emojis_et, std::size(emojis_et)},
    {"fi", emojis_fi, std::size(emojis_fi)},
    {"fr", emojis_fr, std::size(emojis_fr)},
    {"hi", emojis_hi, std::size(emojis_hi)},
    {"hu", emojis_hu, std::size(emojis_hu)},
    {"it", emojis_it, std::size(emojis_it)},
    {"ja", emojis_ja, std::size(emojis_ja)},
    {"ko", emojis_ko, std::size(emojis_ko)},
    {"lt", emojis_lt, std::size(emojis_lt)},
    {"ms", emojis_ms, std::size(emojis_ms)},
    {"nb", emojis_nb, std::size(emojis_nb)},
    {"nl", emojis_nl, std::size(emojis_nl)},
    {"pl", emojis_pl, std::size(emojis_pl)},
    {"pt", emojis_pt, std::size(emojis_pt)},
    {"ru", emojis_ru, std::size(emojis_ru)},
    {"sv", emojis_sv, std::size(emojis_sv)},
    {"th", emojis_th, std::size(emojis_th)},
    {"uk", emojis_uk, std::size(emojis_uk)},
    {"vi", emojis_vi, std::size(emojis_vi)},
    {"zh", emojis_zh, std::size(emojis_zh)},
};