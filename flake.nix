{
  description = "Dev environment and package for the Wii U Crash Tester";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    devkitNix = {
      url = "github:bandithedoge/devkitNix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      devkitNix,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs {
          inherit system;
          overlays = [ devkitNix.overlays.default ];
        };
      in
      {
        devShells.default = pkgs.mkShell.override { stdenv = pkgs.devkitNix.stdenvPPC; } { };

        packages.default = pkgs.devkitNix.stdenvPPC.mkDerivation {
          name = "crashtest-0.1.0";
          src = self;

          meta = with pkgs.lib; {
            description = "Wii U homebrew app for testing AromaCrashHandlerModule";
            license = licenses.mit;
          };

          postUnpack = ''
            rm -rf $sourceRoot/build
          '';

          installPhase = ''
            mkdir -p $out
            cp cafecrashtest.wuhb $out
          '';
        };
      }
    );
}
