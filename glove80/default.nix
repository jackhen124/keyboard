# The workflow checks out the pinned MoErgo ZMK source at ../src.
# Adapted from moergo-sc/glove80-zmk-config/config/default.nix.
{ firmware ? import ../src {} }:

let
  config = ./.;
  left = firmware.zmk.override {
    board = "glove80_lh";
    keymap = "${config}/glove80-zmk-layout.keymap";
    kconfig = "${config}/glove80.conf";
  };
  right = firmware.zmk.override {
    board = "glove80_rh";
    keymap = "${config}/glove80-zmk-layout.keymap";
    kconfig = "${config}/glove80.conf";
  };
in
firmware.combine_uf2 left right
