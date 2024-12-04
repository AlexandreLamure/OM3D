{
  description = "Back to the 80s";

  inputs = {
    nixpkgs = {
      url = "github:nixos/nixpkgs/nixos-unstable";
    }; flake-utils = {
      url = "github:numtide/flake-utils";
    };
  };
  outputs = { nixpkgs, flake-utils, ... }: flake-utils.lib.eachDefaultSystem (system:
    let
      pkgs = import nixpkgs {
        inherit system;
      };
    in rec {
      devShell = pkgs.mkShell {
        buildInputs = with pkgs; [
          (python3.withPackages(ps: with ps; [
            ipython
            jupyter
            numpy
            scipy
            # sklearn-deap
            matplotlib
            scikit-learn
            scikit-image
            opencv4
          ]))
          glm
          libgcc
          freeglut
          glew
          assimp
          glfw
          xorg.libX11
          pkg-config
          xorg.libX11
          xorg.libXrandr
          xorg.libXinerama
          xorg.libXcursor
          xorg.libXi
        ];
        shellHook = ''
            export LD_LIBRARY_PATH="''${LD_LIBRARY_PATH}''${LD_LIBRARY_PATH:+:}${pkgs.libglvnd}/lib"
            '';
      };
    }
  );
}

