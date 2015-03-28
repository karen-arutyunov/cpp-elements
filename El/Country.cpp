/*
 * Elements - useful abstractions library.
 * Copyright (C) 2005-2015 Karen Arutyunov
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "Country.hpp"

namespace El
{
  const Country Country::null;
  const Country Country::nonexistent(EC_NONEXISTENT);
  const Country Country::nonexistent2(EC_NONEXISTENT2);
  
  const Country::Record Country::countries_[] =
  {
// 001
    { EC_AFG, "AF", "AFG", "004", ".af", Lang::EC_PUS, "Afghanistan" },
    { EC_ALB, "AL", "ALB", "008", ".al", Lang::EC_ALB, "Albania" },
    { EC_DZA, "DZ", "DZA", "012", ".dz", Lang::EC_ARA, "Algeria" },
    { EC_ASM, "AS", "ASM", "016", ".as", Lang::EC_ENG, "American Samoa" },
    { EC_AND, "AD", "AND", "020", ".ad", Lang::EC_SPA, "Andorra" },
    { EC_AGO, "AO", "AGO", "024", ".ao", Lang::EC_POR, "Angola" },
    { EC_AIA, "AI", "AIA", "660", ".ai", Lang::EC_ENG, "Anguilla" },
    { EC_ATA, "AQ", "ATA", "010", "",    Lang::EC_NUL, "Antarctica" },
    { EC_ATG, "AG", "ATG", "028", ".ag", Lang::EC_ENG, "Antigua and Barbuda" },
// 010
    { EC_ARG, "AR", "ARG", "032", ".ar", Lang::EC_SPA, "Argentina" },
    { EC_ARM, "AM", "ARM", "051", ".am", Lang::EC_ARM, "Armenia" },
    { EC_ABW, "AW", "ABW", "533", ".aw", Lang::EC_DUT, "Aruba" },
    { EC_AUS, "AU", "AUS", "036", ".au", Lang::EC_ENG, "Australia" },
    { EC_AUT, "AT", "AUT", "040", ".at", Lang::EC_GER, "Austria" },
    { EC_AZE, "AZ", "AZE", "031", ".az", Lang::EC_AZE, "Azerbaijan" },
    { EC_BHS, "BS", "BHS", "044", ".bs", Lang::EC_ENG, "Bahamas" },
    { EC_BHR, "BH", "BHR", "048", ".bh", Lang::EC_ARA, "Bahrain" },
    { EC_BGD, "BD", "BGD", "050", ".bd", Lang::EC_BEN, "Bangladesh" },
    { EC_BRB, "BB", "BRB", "052", ".bb", Lang::EC_ENG, "Barbados" },
// 020
    { EC_BLR, "BY", "BLR", "112", ".by", Lang::EC_BEL, "Belarus" },
    { EC_BEL, "BE", "BEL", "056", ".be", Lang::EC_GER, "Belgium" },
    { EC_BLZ, "BZ", "BLZ", "084", ".bz", Lang::EC_ENG, "Belize" },
    { EC_BEN, "BJ", "BEN", "204", ".bj", Lang::EC_FRE, "Benin" },
    { EC_BMU, "BM", "BMU", "060", ".bm", Lang::EC_ENG, "Bermuda" },
    { EC_BTN, "BT", "BTN", "064", ".bt", Lang::EC_ENG, "Bhutan" },
    { EC_BOL, "BO", "BOL", "068", ".bo", Lang::EC_SPA, "Bolivia" },
    { EC_BIH, "BA", "BIH", "070", ".ba", Lang::EC_BOS, "Bosnia and Herzegovina" },
    { EC_BWA, "BW", "BWA", "072", ".bw", Lang::EC_ENG, "Botswana" },
    { EC_BVT, "BV", "BVT", "074", "",    Lang::EC_NUL, "Bouvet Island" },
// 030
    { EC_BRA, "BR", "BRA", "076", ".br", Lang::EC_POR, "Brazil" },
    { EC_IOT, "IO", "IOT", "086", "",    Lang::EC_ENG, "British Indian Ocean Territory" },
    { EC_VGB, "VG", "VGB", "092", ".vg", Lang::EC_ENG, "British Virgin Islands" },
    { EC_BRN, "BN", "BRN", "096", ".bn", Lang::EC_MAY, "Brunei Darussalam" },
    { EC_BGR, "BG", "BGR", "100", ".bg", Lang::EC_BUL, "Bulgaria" },
    { EC_BFA, "BF", "BFA", "854", ".bf", Lang::EC_FRE, "Burkina Faso" },
    { EC_BDI, "BI", "BDI", "108", ".bi", Lang::EC_FRE, "Burundi" },
    { EC_KHM, "KH", "KHM", "116", ".kh", Lang::EC_KHM, "Cambodia" },
    { EC_CMR, "CM", "CMR", "120", ".cm", Lang::EC_FRE, "Cameroon" },
    { EC_CAN, "CA", "CAN", "124", ".ca", Lang::EC_ENG, "Canada" },
// 040
    { EC_CPV, "CV", "CPV", "132", ".cv", Lang::EC_POR, "Cape Verde" },
    { EC_CYM, "KY", "CYM", "136", ".ky", Lang::EC_ENG, "Cayman Islands" },
    { EC_CAF, "CF", "CAF", "140", ".cf", Lang::EC_SAG, "Central African Republic" },
    { EC_TCD, "TD", "TCD", "148", ".td", Lang::EC_FRE, "Chad" },
    { EC_CHL, "CL", "CHL", "152", ".cl", Lang::EC_SPA, "Chile" },
    { EC_CHN, "CN", "CHN", "156", ".cn", Lang::EC_CHI, "China" },
    { EC_CXR, "CX", "CXR", "162", "",    Lang::EC_ENG, "Christmas Island" },
    { EC_CCK, "CC", "CCK", "166", "",    Lang::EC_ENG, "Cocos Islands" },
    { EC_COL, "CO", "COL", "170", ".co", Lang::EC_SPA, "Colombia" },
    { EC_COM, "KM", "COM", "174", ".km", Lang::EC_FRE, "Comoros" },
// 050
    { EC_COD, "CD", "COD", "180", ".cd", Lang::EC_FRE, "Congo, Democratic Republic of" },
    { EC_COG, "CG", "COG", "178", ".cg", Lang::EC_FRE, "Congo, People's Republic of" },
    { EC_COK, "CK", "COK", "184", ".ck", Lang::EC_ENG, "Cook Islands" },
    { EC_CRI, "CR", "CRI", "188", ".cr", Lang::EC_SPA, "Costa Rica" },
    { EC_CIV, "CI", "CIV", "384", ".ci", Lang::EC_FRE, "Cote D'Ivoire" },
    { EC_CUB, "CU", "CUB", "192", ".cu", Lang::EC_SPA, "Cuba" },
    { EC_CYP, "CY", "CYP", "196", ".cy", Lang::EC_GRE, "Cyprus" },
    { EC_CZE, "CZ", "CZE", "203", ".cz", Lang::EC_CZE, "Czech Republic" },
    { EC_DNK, "DK", "DNK", "208", ".dk", Lang::EC_DAN, "Denmark" },
    { EC_DJI, "DJ", "DJI", "262", ".dj", Lang::EC_FRE, "Djibouti" },
// 060
    { EC_DMA, "DM", "DMA", "212", ".dm", Lang::EC_ENG, "Dominica" },
    { EC_DOM, "DO", "DOM", "214", ".do", Lang::EC_SPA, "Dominican Republic" },
    { EC_ECU, "EC", "ECU", "218", ".ec", Lang::EC_SPA, "Ecuador" },
    { EC_EGY, "EG", "EGY", "818", ".eg", Lang::EC_ARA, "Egypt" },
    { EC_SLV, "SV", "SLV", "222", ".sv", Lang::EC_SPA, "El Salvador" },
    { EC_GNQ, "GQ", "GNQ", "226", ".gq", Lang::EC_POR, "Equatorial Guinea" },
    { EC_ERI, "ER", "ERI", "232", ".er", Lang::EC_TIR, "Eritrea" },
    { EC_EST, "EE", "EST", "233", ".ee", Lang::EC_EST, "Estonia" },
    { EC_ETH, "ET", "ETH", "231", ".et", Lang::EC_AMH, "Ethiopia" },
    { EC_FRO, "FO", "FRO", "234", ".fo", Lang::EC_FAO, "Faeroe Islands" },
// 070
    { EC_FLK, "FK", "FLK", "238", ".fk", Lang::EC_ENG, "Falkland Islands" },
    { EC_FJI, "FJ", "FJI", "242", ".fj", Lang::EC_FIJ, "Fiji" },
    { EC_FIN, "FI", "FIN", "246", ".fi", Lang::EC_FIN, "Finland" },
    { EC_FRA, "FR", "FRA", "250", ".fr", Lang::EC_FRE, "France" },
    { EC_GUF, "GF", "GUF", "254", ".gf", Lang::EC_FRE, "French Guiana" },
    { EC_PYF, "PF", "PYF", "258", ".pf", Lang::EC_FRE, "French Polynesia" },
    { EC_ATF, "TF", "ATF", "260", "",    Lang::EC_FRE, "French Southern Territories" },
    { EC_GAB, "GA", "GAB", "266", ".ga", Lang::EC_FRE, "Gabon" },
    { EC_GMB, "GM", "GMB", "270", ".gm", Lang::EC_ENG, "Gambia" },
    { EC_GEO, "GE", "GEO", "268", ".ge", Lang::EC_GEO, "Georgia" },
// 080
    { EC_DEU, "DE", "DEU", "276", ".de", Lang::EC_GER, "Germany" },
    { EC_GHA, "GH", "GHA", "288", ".gh", Lang::EC_ENG, "Ghana" },
    { EC_GIB, "GI", "GIB", "292", ".gi", Lang::EC_ENG, "Gibraltar" },
    { EC_GRC, "GR", "GRC", "300", ".gr", Lang::EC_GRE, "Greece" },
    { EC_GRL, "GL", "GRL", "304", ".gl", Lang::EC_KAL, "Greenland" },
    { EC_GRD, "GD", "GRD", "308", ".gd", Lang::EC_ENG, "Grenada" },
    { EC_GLP, "GP", "GLP", "312", ".gp", Lang::EC_FRE, "Guadaloupe" },
    { EC_GUM, "GU", "GUM", "316", ".gu", Lang::EC_CHA, "Guam" },
    { EC_GTM, "GT", "GTM", "320", ".gt", Lang::EC_SPA, "Guatemala" },
    { EC_GIN, "GN", "GIN", "324", ".gn", Lang::EC_FRE, "Guinea" },
// 090
    { EC_GNB, "GW", "GNB", "624", ".gw", Lang::EC_POR, "Guinea-Bissau" },
    { EC_GUY, "GY", "GUY", "328", ".gy", Lang::EC_ENG, "Guyana" },
    { EC_HTI, "HT", "HTI", "332", ".ht", Lang::EC_FRE, "Haiti" },
    { EC_HMD, "HM", "HMD", "334", "",    Lang::EC_ENG, "Heard and McDonald Islands" },
    { EC_VAT, "VA", "VAT", "336", ".va", Lang::EC_ITA, "Vatican" },
    { EC_HND, "HN", "HND", "340", ".hn", Lang::EC_SPA, "Honduras" },
    { EC_HKG, "HK", "HKG", "344", ".hk", Lang::EC_CHI, "Hong Kong" },
    { EC_HRV, "HR", "HRV", "191", ".hr", Lang::EC_SCR, "Hrvatska" },
    { EC_HUN, "HU", "HUN", "348", ".hu", Lang::EC_HUN, "Hungary" },
    { EC_ISL, "IS", "ISL", "352", ".is", Lang::EC_ICE, "Iceland" },
// 100
    { EC_IND, "IN", "IND", "356", ".in", Lang::EC_HIN, "India" },
    { EC_IDN, "ID", "IDN", "360", ".id", Lang::EC_IND, "Indonesia" },
    { EC_IRN, "IR", "IRN", "364", ".ir", Lang::EC_PER, "Iran" },
    { EC_IRQ, "IQ", "IRQ", "368", ".iq", Lang::EC_ARA, "Iraq" },
    { EC_IRL, "IE", "IRL", "372", ".ie", Lang::EC_GLE, "Ireland" },
    { EC_ISR, "IL", "ISR", "376", ".il", Lang::EC_HEB, "Israel" },
    { EC_ITA, "IT", "ITA", "380", ".it", Lang::EC_ITA, "Italy" },
    { EC_JAM, "JM", "JAM", "388", ".jm", Lang::EC_ENG, "Jamaica" },
    { EC_JPN, "JP", "JPN", "392", ".jp", Lang::EC_JPN, "Japan" },
    { EC_JOR, "JO", "JOR", "400", ".jo", Lang::EC_ARA, "Jordan" },
// 110
    { EC_KAZ, "KZ", "KAZ", "398", ".kz", Lang::EC_KAZ, "Kazakhstan" },
    { EC_KEN, "KE", "KEN", "404", ".ke", Lang::EC_SWA, "Kenya" },
    { EC_KIR, "KI", "KIR", "296", ".ki", Lang::EC_ENG, "Kiribati" },
    { EC_PRK, "KP", "PRK", "408", ".kp", Lang::EC_KOR, "Korea, Democratic People's Republic of" },
    { EC_KOR, "KR", "KOR", "410", ".kr", Lang::EC_KOR, "Korea, Republic of" },
    { EC_KWT, "KW", "KWT", "414", ".kw", Lang::EC_ARA, "Kuwait" },
    { EC_KGZ, "KG", "KGZ", "417", ".kg", Lang::EC_KIR, "Kyrgyz Republic" },
    { EC_LAO, "LA", "LAO", "418", ".la", Lang::EC_LAO, "Lao" },
    { EC_LVA, "LV", "LVA", "428", ".lv", Lang::EC_LAV, "Latvia" },
    { EC_LBN, "LB", "LBN", "422", "",    Lang::EC_ARA, "Lebanon" },
// 120
    { EC_LSO, "LS", "LSO", "426", ".ls", Lang::EC_SOT, "Lesotho" },
    { EC_LBR, "LR", "LBR", "430", ".lr", Lang::EC_ENG, "Liberia" },
    { EC_LBY, "LY", "LBY", "434", ".ly", Lang::EC_ARA, "Libyan" },
    { EC_LIE, "LI", "LIE", "438", ".li", Lang::EC_GER, "Liechtenstein" },
    { EC_LTU, "LT", "LTU", "440", ".lt", Lang::EC_LIT, "Lithuania" },
    { EC_LUX, "LU", "LUX", "442", ".lu", Lang::EC_GER, "Luxembourg" },
    { EC_MAC, "MO", "MAC", "446", ".mo", Lang::EC_CHI, "Macao" },
    { EC_MKD, "MK", "MKD", "807", ".mk", Lang::EC_MAC, "Macedonia" },
    { EC_MDG, "MG", "MDG", "450", ".mg", Lang::EC_MLG, "Madagascar" },
    { EC_MWI, "MW", "MWI", "454", ".mw", Lang::EC_NYA, "Malawi" },
// 130
    { EC_MYS, "MY", "MYS", "458", ".my", Lang::EC_MAY, "Malaysia" },
    { EC_MDV, "MV", "MDV", "462", ".mv", Lang::EC_DIV, "Maldives" },
    { EC_MLI, "ML", "MLI", "466", ".ml", Lang::EC_FRE, "Mali" },
    { EC_MLT, "MT", "MLT", "470", ".mt", Lang::EC_MLT, "Malta" },
    { EC_MHL, "MH", "MHL", "584", ".mh", Lang::EC_MAH, "Marshall Islands" },
    { EC_MTQ, "MQ", "MTQ", "474", ".mq", Lang::EC_FRE, "Martinique" },
    { EC_MRT, "MR", "MRT", "478", ".mr", Lang::EC_ARA, "Mauritania" },
    { EC_MUS, "MU", "MUS", "480", ".mu", Lang::EC_ENG, "Mauritius" },
    { EC_MYT, "YT", "MYT", "175", "",    Lang::EC_FRE, "Mayotte" },
    { EC_MEX, "MX", "MEX", "484", ".mx", Lang::EC_SPA, "Mexico" },
// 140
    { EC_FSM, "FM", "FSM", "583", ".fm", Lang::EC_CHA, "Micronesia" },
    { EC_MDA, "MD", "MDA", "498", ".md", Lang::EC_MOL, "Moldova" },
    { EC_MCO, "MC", "MCO", "492", ".mc", Lang::EC_FRE, "Monaco" },
    { EC_MNG, "MN", "MNG", "496", ".mn", Lang::EC_MON, "Mongolia" },
    { EC_MSR, "MS", "MSR", "500", ".ms", Lang::EC_ENG, "Montserrat" },
    { EC_MAR, "MA", "MAR", "504", ".ma", Lang::EC_ARA, "Morocco" },
    { EC_MOZ, "MZ", "MOZ", "508", ".mz", Lang::EC_POR, "Mozambique" },
    { EC_MMR, "MM", "MMR", "104", ".mm", Lang::EC_BUR, "Myanmar" },
    { EC_NAM, "NA", "NAM", "516", ".na", Lang::EC_ENG, "Namibia" },
    { EC_NRU, "NR", "NRU", "520", ".nr", Lang::EC_NAU, "Nauru" },
// 150
    { EC_NPL, "NP", "NPL", "524", ".np", Lang::EC_NEP, "Nepal" },
    { EC_ANT, "AN", "ANT", "530", ".an", Lang::EC_DUT, "Netherlands Antilles" },
    { EC_NLD, "NL", "NLD", "528", ".nl", Lang::EC_DUT, "Netherlands" },
    { EC_NCL, "NC", "NCL", "540", ".nc", Lang::EC_FRE, "New Caledonia" },
    { EC_NZL, "NZ", "NZL", "554", ".nz", Lang::EC_MAO, "New Zealand" },
    { EC_NIC, "NI", "NIC", "558", ".ni", Lang::EC_SPA, "Nicaragua" },
    { EC_NER, "NE", "NER", "562", ".ne", Lang::EC_FRE, "Niger" },
    { EC_NGA, "NG", "NGA", "566", ".ng", Lang::EC_FRE, "Nigeria" },
    { EC_NIU, "NU", "NIU", "570", ".nu", Lang::EC_ENG, "Niue" },
    { EC_NFK, "NF", "NFK", "574", ".nf", Lang::EC_ENG, "Norfolk Island" },
// 160
    { EC_MNP, "MP", "MNP", "580", ".mp", Lang::EC_ENG, "Northern Mariana Islands" },
    { EC_NOR, "NO", "NOR", "578", ".no", Lang::EC_NOR, "Norway" },
    { EC_OMN, "OM", "OMN", "512", ".om", Lang::EC_ARA, "Oman" },
    { EC_PAK, "PK", "PAK", "586", ".pk", Lang::EC_URD, "Pakistany" },
    { EC_PLW, "PW", "PLW", "585", ".pw", Lang::EC_ENG, "Palau" },
    { EC_PSE, "PS", "PSE", "275", ".ps", Lang::EC_ARA, "Palestinian Territory" },
    { EC_PAN, "PA", "PAN", "591", ".pa", Lang::EC_SPA, "Panama" },
    { EC_PNG, "PG", "PNG", "598", ".pg", Lang::EC_ENG, "Papua New Guinea" },
    { EC_PRY, "PY", "PRY", "600", ".py", Lang::EC_GRN, "Paraguay" },
    { EC_PER, "PE", "PER", "604", ".pe", Lang::EC_QUE, "Peru" },
// 170
    { EC_PHL, "PH", "PHL", "608", ".ph", Lang::EC_TGL, "Philippines" },
    { EC_PCN, "PN", "PCN", "612", ".pn", Lang::EC_ENG, "Pitcairn Island" },
    { EC_POL, "PL", "POL", "616", ".pl", Lang::EC_POL, "Poland" },
    { EC_PRT, "PT", "PRT", "620", ".pt", Lang::EC_POR, "Portugal" },
    { EC_PRI, "PR", "PRI", "630", ".pr", Lang::EC_SPA, "Puerto Rico" },
    { EC_QAT, "QA", "QAT", "634", ".qa", Lang::EC_ARA, "Qatar" },
    { EC_REU, "RE", "REU", "638", ".re", Lang::EC_FRE, "Reunion" },
    { EC_ROU, "RO", "ROU", "642", ".ro", Lang::EC_RUM, "Romania" },
    { EC_RUS, "RU", "RUS", "643", ".ru", Lang::EC_RUS, "Russia" },
    { EC_RWA, "RW", "RWA", "646", ".rw", Lang::EC_KIN, "Rwanda" },
// 180
    { EC_SHN, "SH", "SHN", "654", ".sh", Lang::EC_ENG, "St. Helena" },
    { EC_KNA, "KN", "KNA", "659", ".kn", Lang::EC_ENG, "St. Kitts and Nevis" },
    { EC_LCA, "LC", "LCA", "662", ".lc", Lang::EC_ENG, "St. Lucia" },
    { EC_SPM, "PM", "SPM", "666", ".pm", Lang::EC_FRE, "St. Pierre and Miquelon" },
    { EC_VCT, "VC", "VCT", "670", ".vc", Lang::EC_ENG, "St. Vincent and the Grenadines" },
    { EC_WSM, "WS", "WSM", "882", ".ws", Lang::EC_SMO, "Samoa" },
    { EC_SMR, "SM", "SMR", "674", ".sm", Lang::EC_ITA, "San Marino" },
    { EC_STP, "ST", "STP", "678", ".st", Lang::EC_POR, "Sao Tome and Principe" },
    { EC_SAU, "SA", "SAU", "682", ".sa", Lang::EC_ARA, "Saudi Arabia" },
    { EC_SEN, "SN", "SEN", "686", ".sn", Lang::EC_WOL, "Senegal" },
// 190
    { EC_SCG, "CS", "SCG", "891", ".rs", Lang::EC_SCC, "Serbia and Montenegro" },
    { EC_SYC, "SC", "SYC", "690", ".sc", Lang::EC_FRE, "Seychelles" },
    { EC_SLE, "SL", "SLE", "694", ".sl", Lang::EC_ENG, "Sierra Leone" },
    { EC_SGP, "SG", "SGP", "702", ".sg", Lang::EC_MAY, "Singapore" },
    { EC_SVK, "SK", "SVK", "703", ".sk", Lang::EC_SLO, "Slovakia" },
    { EC_SVN, "SI", "SVN", "705", ".si", Lang::EC_SLV, "Slovenia" },
    { EC_SLB, "SB", "SLB", "090", ".sb", Lang::EC_ENG, "Solomon Islands" },
    { EC_SOM, "SO", "SOM", "706", ".so", Lang::EC_SOM, "Somalia" },
    { EC_ZAF, "ZA", "ZAF", "710", ".za", Lang::EC_ENG, "South Africa" },
    { EC_SGS, "GS", "SGS", "239", "",    Lang::EC_ENG, "South Georgia and the South Sandwich Islands" },
// 200
    { EC_ESP, "ES", "ESP", "724", ".es", Lang::EC_SPA, "Spain" },
    { EC_LKA, "LK", "LKA", "144", ".lk", Lang::EC_SIN, "Sri Lanka" },
    { EC_SDN, "SD", "SDN", "736", ".sd", Lang::EC_ARA, "Sudan" },
    { EC_SUR, "SR", "SUR", "740", ".sr", Lang::EC_DUT, "Suriname" },
    { EC_SJM, "SJ", "SJM", "744", ".sj", Lang::EC_NOR, "Svalbard & Jan Mayen Islands" },
    { EC_SWZ, "SZ", "SWZ", "748", ".sz", Lang::EC_SSW, "Swaziland" },
    { EC_SWE, "SE", "SWE", "752", ".se", Lang::EC_SWE, "Sweden" },
    { EC_CHE, "CH", "CHE", "756", ".ch", Lang::EC_GER, "Switzerland" },
    { EC_SYR, "SY", "SYR", "760", ".sy", Lang::EC_ARA, "Syrian Arab Republic" },
    { EC_TWN, "TW", "TWN", "158", ".tw", Lang::EC_CHI, "Taiwan" },
// 210
    { EC_TJK, "TJ", "TJK", "762", ".tj", Lang::EC_TGK, "Tajikistan" },
    { EC_TZA, "TZ", "TZA", "834", ".tz", Lang::EC_SWA, "Tanzania" },
    { EC_THA, "TH", "THA", "764", ".th", Lang::EC_THA, "Thailand" },
    { EC_TLS, "TL", "TLS", "626", "",    Lang::EC_POR, "Timor-Leste" },
    { EC_TGO, "TG", "TGO", "768", ".tg", Lang::EC_FRE, "Togo" },
    { EC_TKL, "TK", "TKL", "772", ".tk", Lang::EC_ENG, "Tokelau" },
    { EC_TON, "TO", "TON", "776", ".to", Lang::EC_TON, "Tonga" },
    { EC_TTO, "TT", "TTO", "780", ".tt", Lang::EC_ENG, "Trinidad and Tobago" },
    { EC_TUN, "TN", "TUN", "788", ".tn", Lang::EC_ARA, "Tunisia" },
    { EC_TUR, "TR", "TUR", "792", ".tr", Lang::EC_TUR, "Turkey" },

// 220
    { EC_TKM, "TM", "TKM", "795", ".tm", Lang::EC_TUK, "Turkmenistan" },
    { EC_TCA, "TC", "TCA", "796", ".tc", Lang::EC_ENG, "Turks and Caicos Islands" },
    { EC_TUV, "TV", "TUV", "798", ".tv", Lang::EC_ENG, "Tuvalu" },
    { EC_VIR, "VI", "VIR", "850", ".vi", Lang::EC_ENG, "US Virgin Islands" },
    { EC_UGA, "UG", "UGA", "800", ".ug", Lang::EC_SWA, "Uganda" },
    { EC_UKR, "UA", "UKR", "804", ".ua", Lang::EC_UKR, "Ukraine" },
    { EC_ARE, "AE", "ARE", "784", ".ae", Lang::EC_ARA, "United Arab Emirates" },
    { EC_GBR, "GB", "GBR", "826", ".uk", Lang::EC_ENG, "United Kingdom of Great Britain & N. Ireland" },
    { EC_UMI, "UM", "UMI", "581", "",    Lang::EC_ENG, "United States Minor Outlying Islands" },
    { EC_USA, "US", "USA", "840", ".us", Lang::EC_ENG, "United States of America" },

// 230
    { EC_URY, "UY", "URY", "858", ".uy", Lang::EC_SPA, "Uruguay" },
    { EC_UZB, "UZ", "UZB", "860", ".uz", Lang::EC_UZB, "Uzbekistan" },
    { EC_VUT, "VU", "VUT", "548", ".vu", Lang::EC_BIS, "Vanuatu" },
    { EC_VEN, "VE", "VEN", "862", ".ve", Lang::EC_SPA, "Venezuela" },
    { EC_VNM, "VN", "VNM", "704", ".vn", Lang::EC_VIE, "Viet Nam" },
    { EC_WLF, "WF", "WLF", "876", ".wf", Lang::EC_FRE, "Wallis and Futuna Islands" },
    { EC_ESH, "EH", "ESH", "732", ".eh", Lang::EC_ARA, "Western Sahara" },
    { EC_YEM, "YE", "YEM", "887", ".ye", Lang::EC_ARA, "Yemen" },
    { EC_ZMB, "ZM", "ZMB", "894", ".zm", Lang::EC_ENG, "Zambia" },
    { EC_ZWE, "ZW", "ZWE", "716", ".zw", Lang::EC_ENG, "Zimbabwe" },
// 240
    { EC_GBR, "GB", "GBR", "826", ".gb",  Lang::EC_ENG, "United Kingdom of Great Britain & N. Ireland" },
    { EC_USA, "US", "USA", "840", ".edu", Lang::EC_ENG, "United States of America" },
    { EC_USA, "US", "USA", "840", ".gov", Lang::EC_ENG, "United States of America" },
    { EC_USA, "US", "USA", "840", ".mil", Lang::EC_ENG, "United States of America" },
    { EC_SCG, "CS", "SCG", "891", ".yu",  Lang::EC_SCC, "Serbia and Montenegro" },
    
    { EC_NUL, "ZZ", "ZZZ", "999", "", Lang::EC_NUL, "null" }
  };

  const unsigned long Country::countries_count_ =
    sizeof(Country::countries_) / sizeof(Country::countries_[0]) - 1 - 5;
}