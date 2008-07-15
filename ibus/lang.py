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

__all__ = (
		"LANGUAGES",
	)

N_ = lambda a: a

LANGUAGES = {
	"C"  : N_("English/Keyboard"),
    "am_ET"  : N_("Amharic"),
    "ar"  : N_("Arabic"),
    "ar_EG"  : N_("Arabic (Egypt)"),
    "ar_LB"  : N_("Arabic (Lebanon)"),
    "as_IN"  : N_("Assamese"),
    "az_AZ"  : N_("Azerbaijani"),
    "be_BY"  : N_("Belarusian"),
    "bg_BG"  : N_("Bulgarian"),
    "bn"  : N_("Bengali"),
    "bn_BD"  : N_("Bengali"),
    "bn_IN"  : N_("Bengali (India)"),
    "bo"  : N_("Tibetan"),
    "bs_BA"  : N_("Bosnian"),
    "ca_ES"  : N_("Catalan"),
    "cs_CZ"  : N_("Czech"),
    "cy_GB"  : N_("Welsh"),
    "da_DK"  : N_("Danish"),
    "de_DE"  : N_("German"),
    "dv_MV"  : N_("Divehi"),
    "el_GR"  : N_("Greek"),
    "en"     : N_("English"),
    "en_AU"  : N_("English (Australian)"),
    "en_CA"  : N_("English (Canadian)"),
    "en_GB"  : N_("English (British)"),
    "en_IE"  : N_("English (Ireland)"),
    "en_US"  : N_("English (American)"),
    "es"  : N_("Spanish"),
    "es_ES"  : N_("Spanish"),
    "es_MX"  : N_("Spanish (Mexico)"),
    "et_EE"  : N_("Estonian"),
    "eu_ES"  : N_("Basque"),
    "fa_IR"  : N_("Persian"),
    "fi_FI"  : N_("Finnish"),
    "fr_FR"  : N_("French"),
    "ga_IE"  : N_("Irish"),
    "gl_ES"  : N_("Galician"),
    "gu_IN"  : N_("Gujarati"),
    "he_IL"  : N_("Hebrew"),
    "hi_IN"  : N_("Hindi"),
    "hr_HR"  : N_("Croatian"),
    "hu_HU"  : N_("Hungarian"),
    "hy_AM"  : N_("Armenian"),
    "ia"     : N_("Interlingua"),
    "id_ID"  : N_("Indonesian"),
    "is_IS"  : N_("Icelandic"),
    "it_IT"  : N_("Italian"),
    "iw_IL"  : N_("Hebrew"),
    "ja_JP"  : N_("Japanese"),
    "ka_GE"  : N_("Georgian"),
    "kk_KZ"  : N_("Kazakh"),
    "km"  : N_("Cambodian"),
    "kn_IN"  : N_("Kannada"),
    "ko_KR"  : N_("Korean"),
    "lo_LA"  : N_("Laothian"),
    "lt_LT"  : N_("Lithuanian"),
    "lv_LV"  : N_("Latvian"),
    "mk_MK"  : N_("Macedonian"),
    "ml_IN"  : N_("Malayalam"),
    "mn_MN"  : N_("Mongolian"),
    "mr_IN"  : N_("Marathi"),
    "ms_MY"  : N_("Malay"),
    "my_MM"  : N_("Burmese"),
    "ne_NP"  : N_("Nepali"),
    "nl_NL"  : N_("Dutch"),
    "nn_NO"  : N_("Norwegian (nynorsk)"),
    "no_NO"  : N_("Norwegian (bokmal)"),
    "or_IN"  : N_("Oriya"),
    "pa_IN"  : N_("Punjabi"),
    "pl_PL"  : N_("Polish"),
    "pt"  : N_("Portuguese"),
    "pt_BR"  : N_("Portuguese (Brazil)"),
    "pt_PT"  : N_("Portuguese"),
    "ro_RO"  : N_("Romanian"),
    "ru_RU"  : N_("Russian"),
    "si_LK"  : N_("Sinhala"),
    "sk_SK"  : N_("Slovak"),
    "sl_SI"  : N_("Slovenian"),
    "sq_AL"  : N_("Albanian"),
    "sr"  : N_("Serbian"),
    "sr_CS"  : N_("Serbian"),
    "sr_YU"  : N_("Serbian"),
    "sv"  : N_("Swedish"),
    "sv_FI"  : N_("Swedish (Finland)"),
    "sv_SE"  : N_("Swedish"),
    "ta_IN"  : N_("Tamil"),
    "te_IN"  : N_("Telugu"),
    "th_TH"  : N_("Thai"),
    "tr_TR"  : N_("Turkish"),
    "ug"  : N_("Uighur"),
    "uk_UA"  : N_("Ukrainian"),
    "ur_PK"  : N_("Urdu"),
    "uz_UZ"  : N_("Uzbek"),
    "vi_VN"  : N_("Vietnamese"),
    "wa_BE"  : N_("Walloon"),
    "yi"     : N_("Yiddish"),
    "yi_US"  : N_("Yiddish"),
    "zh"  : N_("Chinese"),
    "zh_CN"  : N_("Chinese (simplified)"),
    "zh_HK"  : N_("Chinese (traditional)"),
    "zh_TW"  : N_("Chinese (traditional)"),
}

for k in LANGUAGES.keys():
	try:
		lang, local = k.split("_")
		if lang not in LANGUAGES:
			LANGUAGES[lang] = LANGUAGES[k]
	except:
		pass
