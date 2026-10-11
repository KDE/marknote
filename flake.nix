{
  description = "Development environment with KDE Frameworks 6 and md4qt";

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
    in
    {
      packages = forAllSystems (system:
        let
          pkgs = import nixpkgs { inherit system; };
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

            # Tell qtPreHook that this is a library and does not require binary wrapping
            dontWrapQtApps = true;

            cmakeFlags = [
              "-DBUILD_TESTING=OFF"
            ];
          };
        });

      devShells = forAllSystems (system:
        let
          pkgs = import nixpkgs { inherit system; };
          md4qt = self.packages.${system}.md4qt;
        in
        {
          default = pkgs.mkShell {
            nativeBuildInputs = with pkgs; [
              cmake
              ninja
              pkg-config
              kdePackages.extra-cmake-modules
              kdePackages.wrapQtAppsHook
            ];

            buildInputs = (with pkgs.kdePackages; [
              qtbase
              qtdeclarative
              kconfig
              ki18n
              kiconthemes
              kirigami
              kirigami-addons
              kcoreaddons
              kcolorscheme
              knotifications
              syntax-highlighting
              kmime
              kitemmodels
              kwindowsystem
              kcrash
            ]) ++ [
              md4qt
            ];

            shellHook = ''
              export CMAKE_PREFIX_PATH="${md4qt}:$CMAKE_PREFIX_PATH"
              echo "KF6 development environment ready."
            '';
          };
        });
    };
}
