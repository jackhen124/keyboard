# Voyager features are defined in the keyboard's data-driven config.
MOUSEKEY_ENABLE = yes   # required for the _MOUSE layer (KC_MS_*, KC_BTN*, KC_WH_*)

# RGB lighting lives in its own module (voyager/flash/lighting/), separated by
# concern: controller (state + selector keys), patterns (brightness), colors
# (hue), and the render loop that combines them. keymap.c only includes
# lighting/lighting.h.
SRC += lighting/lighting.c
SRC += lighting/patterns.c
SRC += lighting/colors.c
SRC += lighting/render.c

# Add optional QMK features here if you extend the keymap, e.g.:
# COMBO_ENABLE = yes
# TAP_DANCE_ENABLE = yes
# UNICODE_ENABLE = yes
