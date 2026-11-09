# SPDX-FileCopyrightText: 2025 Prayag Jain <prayagjain2@gmail.com>
#
# SPDX-License-Identifier: GPL-3.0-or-later

{
  description = "Marknote package and development environment with KDE Frameworks 6 and md4qt";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    md4qt-src = {
      url = "git+https://invent.kde.org/libraries/md4qt.git";
      flake = false;
    };
  };

  outputs = { self, nixpkgs, md4qt-src }:
    let
      supportedSystems = [ "x86_64-linux" "aarch64-linux" ];
      forAllSystems = nixpkgs.lib.genAttrs supportedSystems;

      kdeDepsFor = pkgs: with pkgs.kdePackages; [
        qtbase
        qtdeclarative
        qtwayland
        qtsvg
        kconfig
        ki18n
        kiconthemes
        kcoreaddons
        kcolorscheme
        knotifications
        syntax-highlighting
        kmime
        kitemmodels
        kwindowsystem
        kcrash
        kirigami
        kirigami-addons
        kdeclarative
        sonnet
      ];
    in
    {
      packages = forAllSystems (system:
        let
          pkgs = import nixpkgs { inherit system; };
          marknote = pkgs.stdenv.mkDerivation {
            pname = "marknote";
            version = "1.6.0";
            src = ./.;

            nativeBuildInputs = [
              pkgs.cmake
              pkgs.ninja
              pkgs.kdePackages.extra-cmake-modules
              pkgs.kdePackages.wrapQtAppsHook
            ];

            buildInputs = kdeDepsFor pkgs ++ [ self.packages.${system}.md4qt ];

            cmakeFlags = [
              "-DBUILD_TESTING=OFF"
            ];

            meta = {
              license = pkgs.lib.licenses.gpl2Plus;
              mainProgram = "marknote";
              platforms = pkgs.lib.platforms.linux;
            };
          };
        in
        {
          md4qt = pkgs.stdenv.mkDerivation {
            pname = "md4qt";
            version = "latest-kf6";
            src = md4qt-src;

            nativeBuildInputs = [
              pkgs.cmake
              pkgs.ninja
              pkgs.kdePackages.extra-cmake-modules
            ];

            buildInputs = [
              pkgs.kdePackages.qtbase
              pkgs.kdePackages.qtdeclarative
            ];

            # md4qt is a library. The Qt pre-hook errors without an explicit choice.
            dontWrapQtApps = true;

            cmakeFlags = [
              "-DBUILD_TESTING=OFF"
            ];
          };

          inherit marknote;

          default = marknote;
        });

      devShells = forAllSystems (system:
        let
          pkgs = import nixpkgs { inherit system; };
          md4qt = self.packages.${system}.md4qt;
          qmlRoots = kdeDepsFor pkgs;
          
          # wrapQtAppsHook appends each Qt dependency's lib/qt-6/qml and
          # lib/qt-6/plugins directories to qtWrapperArgs, direct or
          # propagated. The args are only consumed when a build wraps
          # binaries, which mkShell never does. This derivation runs the
          # same collection and re-exports the result for the shell.
          qtRuntimePaths = pkgs.runCommand "marknote-qt-runtime-paths" {
            nativeBuildInputs = [ pkgs.kdePackages.wrapQtAppsHook ];
            buildInputs = qmlRoots;
          } ''
            declare -A merged
            mode="0"
            var=""
            
            # qtWrapperArgs may be a plain string instead of an array.
            if ! declare -p qtWrapperArgs 2>/dev/null | grep -q "^declare -a"; then
              qtWrapperArgs=(''${qtWrapperArgs-})
            fi
            for arg in "''${qtWrapperArgs[@]}"; do
              case $arg in
                --prefix)
                  if [ "$mode" != "0" ]; then
                    echo "unexpected --prefix" >&2
                    exit 1
                  fi
                  mode=1 ;;
                --suffix|--set|--unset|--run)
                  echo "unsupported makeWrapper verb: $arg" >&2
                  exit 1 ;;
                :)
                  if [ "$mode" != "2" ]; then
                    echo "unexpected : separator" >&2
                    exit 1
                  fi
                  mode=3 ;;
                *)
                  if [ "$mode" = "1" ]; then
                    var=$arg
                    mode=2
                  elif [ "$mode" = "3" ]; then
                    if [ ! -d "$arg" ]; then
                      echo "not a directory: $arg" >&2
                      exit 1
                    fi
                    merged[$var]+="''${merged[$var]:+:}$arg"
                    mode=0
                  else
                    echo "unexpected token: $arg" >&2
                    exit 1
                  fi ;;
              esac
            done
            for required in NIXPKGS_QT6_QML_IMPORT_PATH QT_PLUGIN_PATH; do
              if [ -z "''${merged[$required]+x}" ]; then
                echo "qtWrapperArgs yielded no $required" >&2
                exit 1
              fi
            done
            for v in "''${!merged[@]}"; do
              printf 'declare -x %s=%q\n' "$v" "''${merged[$v]}" >> "$out"
            done
          '';
        in
        {
          default = pkgs.mkShell {
            nativeBuildInputs = with pkgs; [
              cmake
              ninja
              pkg-config
              reuse
              kdePackages.extra-cmake-modules
              kdePackages.wrapQtAppsHook
            ];

            buildInputs = qmlRoots ++ [ md4qt ];

            shellHook = ''
              export CMAKE_PREFIX_PATH="${md4qt}''${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}"
              source "${qtRuntimePaths}"
              echo "KF6 development environment ready."
            '';
          };
        });
    };
}
