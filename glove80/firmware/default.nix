# The workflow checks out the pinned MoErgo ZMK source at ../../src.
# Adapted from moergo-sc/glove80-zmk-config/config/default.nix.
{ firmware ? import ../../src {} }:

let
  config = ./.;
  left = firmware.zmk.override {
    board = "glove80_lh";
    keymap = "${config}/layout.keymap";
    kconfig = "${config}/config.conf";
  };
  right = firmware.zmk.override {
    board = "glove80_rh";
    keymap = "${config}/layout.keymap";
    kconfig = "${config}/config.conf";
  };
in
firmware.combine_uf2 left right
