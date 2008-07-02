# vim:set noet ts=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2008 Huang Peng <shawn.p.huang@gmail.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place, Suite 330,
# Boston, MA  02111-1307  USA

LANGUAGES = {
	"C"  : "English/Keyboard",
    "am_ET"  : "Amharic",
    "ar"  : "Arabic",
    "ar_EG"  : "Arabic (Egypt)",
    "ar_LB"  : "Arabic (Lebanon)",
    "as_IN"  : "Assamese",
    "az_AZ"  : "Azerbaijani",
    "be_BY"  : "Belarusian",
    "bg_BG"  : "Bulgarian",
    "bn"  : "Bengali",
    "bn_BD"  : "Bengali",
    "bn_IN"  : "Bengali (India)",
    "bo"  : "Tibetan",
    "bs_BA"  : "Bosnian",
    "ca_ES"  : "Catalan",
    "cs_CZ"  : "Czech",
    "cy_GB"  : "Welsh",
    "da_DK"  : "Danish",
    "de_DE"  : "German",
    "dv_MV"  : "Divehi",
    "el_GR"  : "Greek",
    "en"     : "English",
    "en_AU"  : "English (Australian)",
    "en_CA"  : "English (Canadian)",
    "en_GB"  : "English (British)",
    "en_IE"  : "English (Ireland)",
    "en_US"  : "English (American)",
    "es"  : "Spanish",
    "es_ES"  : "Spanish",
    "es_MX"  : "Spanish (Mexico)",
    "et_EE"  : "Estonian",
    "eu_ES"  : "Basque",
    "fa_IR"  : "Persian",
    "fi_FI"  : "Finnish",
    "fr_FR"  : "French",
    "ga_IE"  : "Irish",
    "gl_ES"  : "Galician",
    "gu_IN"  : "Gujarati",
    "he_IL"  : "Hebrew",
    "hi_IN"  : "Hindi",
    "hr_HR"  : "Croatian",
    "hu_HU"  : "Hungarian",
    "hy_AM"  : "Armenian",
    "ia"     : "Interlingua",
    "id_ID"  : "Indonesian",
    "is_IS"  : "Icelandic",
    "it_IT"  : "Italian",
    "iw_IL"  : "Hebrew",
    "ja_JP"  : "Japanese",
    "ka_GE"  : "Georgian",
    "kk_KZ"  : "Kazakh",
    "km"  : "Cambodian",
    "kn_IN"  : "Kannada",
    "ko_KR"  : "Korean",
    "lo_LA"  : "Laothian",
    "lt_LT"  : "Lithuanian",
    "lv_LV"  : "Latvian",
    "mk_MK"  : "Macedonian",
    "ml_IN"  : "Malayalam",
    "mn_MN"  : "Mongolian",
    "mr_IN"  : "Marathi",
    "ms_MY"  : "Malay",
    "my_MM"  : "Burmese",
    "ne_NP"  : "Nepali",
    "nl_NL"  : "Dutch",
    "nn_NO"  : "Norwegian (nynorsk)",
    "no_NO"  : "Norwegian (bokmal)",
    "or_IN"  : "Oriya",
    "pa_IN"  : "Punjabi",
    "pl_PL"  : "Polish",
    "pt"  : "Portuguese",
    "pt_BR"  : "Portuguese (Brazil)",
    "pt_PT"  : "Portuguese",
    "ro_RO"  : "Romanian",
    "ru_RU"  : "Russian",
    "si_LK"  : "Sinhala",
    "sk_SK"  : "Slovak",
    "sl_SI"  : "Slovenian",
    "sq_AL"  : "Albanian",
    "sr"  : "Serbian",
    "sr_CS"  : "Serbian",
    "sr_YU"  : "Serbian",
    "sv"  : "Swedish",
    "sv_FI"  : "Swedish (Finland)",
    "sv_SE"  : "Swedish",
    "ta_IN"  : "Tamil",
    "te_IN"  : "Telugu",
    "th_TH"  : "Thai",
    "tr_TR"  : "Turkish",
    "ug"  : "Uighur",
    "uk_UA"  : "Ukrainian",
    "ur_PK"  : "Urdu",
    "uz_UZ"  : "Uzbek",
    "vi_VN"  : "Vietnamese",
    "wa_BE"  : "Walloon",
    "yi"     : "Yiddish",
    "yi_US"  : "Yiddish",
    "zh"  : "Chinese",
    "zh_CN"  : "Chinese (simplified)",
    "zh_HK"  : "Chinese (traditional)",
    "zh_TW"  : "Chinese (traditional)",
}

for k in LANGUAGES.keys ():
	try:
		lang, local = k.split ("_")
		if lang not in LANGUAGES:
			LANGUAGES[lang] = LANGUAGES [k]
	except:
		pass
