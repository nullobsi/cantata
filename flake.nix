{
	description = "Cantata development environment";

	inputs = {
		nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

		flake-parts = {
			url = "github:hercules-ci/flake-parts";
			inputs.nixpkgs-lib.follows = "nixpkgs";
		};

		treefmt-nix = {
			url = "github:numtide/treefmt-nix";
			inputs.nixpkgs.follows = "nixpkgs";
		};
	};

	outputs = inputs: inputs.flake-parts.lib.mkFlake { inherit inputs; }
	{
		systems = [
			"x86_64-linux"
			"aarch64-linux"
			"x86_64-darwin"
			"aarch64-darwin"
		];

		perSystem = { config, pkgs, ... }:
		let
			options = if pkgs.stdenv.hostPlatform.isLinux then {} else {
				withUdisks2 = false;
				withMtp = false;
				withLibVlc = false;
				withDevices = false;
				withCdioParanoia = false;
				withMusicbrainz = false;
				withCddb = false;
			};
		in
		{
			packages.default = pkgs.callPackage ./nix/package.nix options;

			devShells.default = pkgs.mkShell {
				inputsFrom = [ config.packages.default ];

				packages = with pkgs; [
					clang-tools
					ninja
				];
			};
		};

		imports = [
			./nix/treefmt.nix
		];
	};
}
