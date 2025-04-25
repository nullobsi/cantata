{
	inputs = {
		nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
		flake-utils.url = "github:numtide/flake-utils";
	};
	outputs = { self, nixpkgs, flake-utils }: flake-utils.lib.eachDefaultSystem (system:
		let
			name = "cantata";
			version = "3.3.1";
			pkgs = import nixpkgs {
				inherit system;
			};
			isLinux = pkgs.lib.strings.hasSuffix "-linux" system;
			qtEnv = with pkgs.qt6; env "qt-custom-${qtbase.version}" 
				([
					qtbase
					qtconnectivity
					qthttpserver
					qtimageformats
					qtmultimedia
					qtsvg
					qttranslations
					qttools
				] ++ pkgs.lib.optionals isLinux [
					qtwayland
				]);
			buildInputs = with pkgs; [
				pkg-config
				taglib
				ffmpeg
				mpg123
				libebur128
				avahi
				zlib
				qtEnv
				cmake
				ninja
			] ++ lib.optionals isLinux [
				libGL
				libGLU
				libcdio
				libcdio-paranoia
				libmusicbrainz5
				libmtp
				media-player-info
			];
		in
		{
			packages.default =
				let
					inherit (pkgs) stdenv lib;
				in
				stdenv.mkDerivation {
					inherit version buildInputs;
					src = self;
					pname = name;
				};
			devShells.default = pkgs.mkShell {
				inherit buildInputs;
			};
		}
	);
	
}
