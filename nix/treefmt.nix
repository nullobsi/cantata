{ inputs, ... }:

{
	imports = [ inputs.treefmt-nix.flakeModule ];

	perSystem =
		{ pkgs, ... }:
		{
			treefmt = {
				projectRootFile = "flake.nix";

				programs.nixfmt.enable = true;
				programs.nixfmt.package = pkgs.nixfmt;

				programs.clang-format.enable = true;
				programs.clang-format.package = pkgs.clang-tools;
			};
		};
}
