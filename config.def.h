#ifndef __DWMSTATUS_CONFIG_H
#define __DWMSTATUS_CONFIG_H

static char *tcanada = "Canada/Eastern";

#define BLACK	"#3a3a3a"
#define RED	"#ef2929"
#define GREEN	"#5faf00"
#define YELLOW	"#ffaf5f"
#define BLUE	"#729fcf"
#define MAGENTA "#ff4ea3"
#define CYAN	"#8cc4ff"
#define WHITE	"#ffffd7"

#define NORMBORDERCOLOR "#212121"
#define NORMBGCOLOR     "#121212"
#define NORMFGCOLOR     "#696969"
#define SELBORDERCOLOR  "#696969"
#define SELBGCOLOR      "#121212"
#define SELFGCOLOR      "#e0e0e0"

#define POWERLINE_HARD_LEFT ""
#define POWERLINE_SOFT_LEFT ""
#define POWERLINE_HARD_RIGHT ""
#define POWERLINE_SOFT_RIGHT ""

#define BATT_NOW "/sys/class/power_supply/BAT0/energy_now"
#define BATT_FULL "/sys/class/power_supply/BAT0/energy_full"
#define BATT_STATUS "/sys/class/power_supply/BAT0/status"

#define WIFI_DEV "wlp1s0"

#define BEER_GLYPH	""
#define COFFEE_GLYPH	""
#define CPU_GLYPH	""
#define EMAIL_GLYPH	""
#define LINUX_GLYPH	""
#define MUSIC_GLYPH	""
#define WIFI_GLYPH	""
#define ETHER_GLYPH     ""

#define CLOCK_GLYPH "<span font=\"Icons\"></span>"
/* #define CLOCK_GLYPH "⌚" */

#define BAT_CHARGING_GLYPH	""
#define BAT_FULL_GLYPH		""
#define BAT_70_PERCENT_GLYPH	""
#define BAT_30_PERCENT_GLYPH	""
#define BAT_EMPTY_GLYPH		""
#define AC_POWER_GLYPH          ""

#endif
