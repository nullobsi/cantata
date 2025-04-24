{
	inputs = {
		nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
		flake-utils.url = "github:numtide/flake-utils";
	};
	outputs = { self, nixpkgs, flake-utils }: flake-utils.lib.eachDefaultSystem (system:
		let
			pkgs = import nixpkgs {
				inherit system;
			};
			qtEnv = with pkgs.qt6; env "qt-custom-${qtbase.version}" 
				[
					qtconnectivity
					qthttpserver
					qtimageformats
					qtmultimedia
					qtsvg
					qttranslations
					qttools
				];
		in
		{
			devShells.default = pkgs.mkShell {
				buildInputs = with pkgs; [
					taglib
					ffmpeg
					mpg123
					libebur128
					avahi
					zlib
					qtEnv
					cmake
					ninja
				];
			};
		}
	);
	
}
