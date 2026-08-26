{
	stdenv,
	lib,

	cmake,
	pkg-config,
	qt6,
	kdePackages,
	perl,
	zlib,

	withCdda ? false,
	cdparanoia,
	withCdioParanoia ? true,
	libcdio,
	libcdio-paranoia,

	withMusicbrainz ? true,
	libmusicbrainz5,
	withCddb ? true,
	libcddb,

	withFFmpeg ? true,
	ffmpeg_6,
	withMPG123 ? true,
	mpg123,
	withReplaygain ? true,
	libebur128,

	withTaglib ? true,
	taglib,
	withHttpStream ? true,
	gst_all_1,
	withMtp ? true,
	libmtp,
	withDevices ? true,
	withUdisks2 ? true,
	udev,
	withHttpServer ? true,
	withLibVlc ? true,
	libvlc,
	withAvahi ? true,
	avahi,
}:

assert withDevices -> withTaglib;
assert withCdioParanoia -> !withCdda && withDevices;
assert withCdda -> !withCdioParanoia && withDevices;
assert withMusicbrainz -> withCdioParanoia || withCdda;
assert withCddb -> withCdioParanoia || withCdda;
assert withFFmpeg -> withTaglib && withReplaygain;
assert withMPG123 -> withTaglib && withReplaygain;
assert withReplaygain -> withFFmpeg || withMPG123;
assert withLibVlc -> withHttpStream && stdenv.hostPlatform.isLinux;
assert withMtp -> withDevices && stdenv.hostPlatform.isLinux;
assert withUdisks2 -> withDevices && stdenv.hostPlatform.isLinux;

let
	gst = with gst_all_1; [
		gstreamer
		gst-libav
		gst-plugins-base
		gst-plugins-good
		gst-plugins-bad
	];

	options = [
		{
			names = [ "ENABLE_CDIOPARANOIA" ];
			enable = withCdioParanoia;
			pkgs = [ libcdio libcdio-paranoia ];
		}
		{
			names = [ "ENABLE_CDPARANOIA" ];
			enable = withCdda;
			pkgs = [ cdparanoia ];
		}
		{
			names = [ "ENABLE_MUSICBRAINZ" ];
			enable = withMusicbrainz;
			pkgs = [ libmusicbrainz5 ];
		}
		{
			names = [ "ENABLE_CDDB" ];
			enable = withCddb;
			pkgs = [ libcddb ];
		}
		{
			names = [ "ENABLE_DEVICES_SUPPORT" ];
			enable = withDevices;
			pkgs = [ ];
		}
		{
			names = [ ];
			enable = withReplaygain;
			pkgs = [ libebur128 ];
		}
		{
			names = [ "ENABLE_FFMPEG" ];
			enable = withFFmpeg;
			pkgs = [ ffmpeg_6 ];
		}
		{
			names = [ "ENABLE_MPG123" ];
			enable = withMPG123;
			pkgs = [ mpg123 ];
		}
		{
			names = [ "ENABLE_HTTP_SERVER" ];
			enable = withHttpServer;
			pkgs = [ ];
		}
		{
			names = [ "ENABLE_HTTP_STREAM_PLAYBACK" ];
			enable = withHttpStream;
			pkgs = lib.optionals (!withLibVlc) [ qt6.qtmultimedia ];
		}
		{
			names = [ "ENABLE_LIBVLC" ];
			enable = withLibVlc;
			pkgs = [ libvlc ];
		}
		{
			names = [ "ENABLE_MTP" ];
			enable = withMtp;
			pkgs = [ libmtp ];
		}
		{
			names = [ "ENABLE_TAGLIB" ];
			enable = withTaglib;
			pkgs = [ taglib ];
		}
		{
			names = [ "ENABLE_UDISKS2" ];
			enable = withUdisks2;
			pkgs = [ udev ];
		}
		{
			names = [ "ENABLE_AVAHI" ];
			enable = withAvahi;
			pkgs = [ avahi ];
		}
	];
in
stdenv.mkDerivation (final: {
	pname = "cantata";
	version = "3.5.0";
	src = ../.;

	patches = [
		./dont-check-for-perl-in-PATH.diff
	];

	postPatch = ''
patchShebangs playlists
'';

	buildInputs = [
		qt6.qtbase
		qt6.qtsvg
		(perl.withPackages (ppkgs: with ppkgs; [ URI ]))
		zlib
	]
	++ lib.flatten (builtins.catAttrs "pkgs" (builtins.filter (e: e.enable) options))
	++ lib.optionals stdenv.hostPlatform.isLinux [
		qt6.qtwayland
		kdePackages.karchive
		kdePackages.kitemviews
	];

	nativeBuildInputs = [
		cmake
		pkg-config
		qt6.qttools
		qt6.wrapQtAppsHook
	];

	cmakeFlags = [
		(lib.cmakeBool "BUNDLED_KCATEGORIZEDVIEW" (!stdenv.hostPlatform.isLinux))
		(lib.cmakeBool "BUNDLED_KARCHIVE" (!stdenv.hostPlatform.isLinux))
		(lib.cmakeBool "BUNDLED_FONTAWESOME" true)
	] ++ lib.flatten (map (e: map (f: lib.cmakeBool f e.enable) e.names) options);

	qtWrapperArgs = lib.optionals (withHttpStream && !withLibVlc) [
		"--prefix GST_PLUGIN_PATH : ${lib.makeSearchPathOutput "lib" "lib/gstreamer-1.0" gst}"
	];

	dontWrapQtApps = stdenv.hostPlatform.isDarwin;

	postInstall = lib.optionalString stdenv.hostPlatform.isDarwin ''
mkdir -p $out/Applications $out/bin
mv $out/Cantata.app $out/Applications/Cantata.app

makeBinaryWrapper $out/Applications/Cantata.app/Contents/MacOS/Cantata $out/bin/cantata
'';

	preFixup = lib.optionalString stdenv.hostPlatform.isDarwin ''
wrapQtApp "$out/Applications/Cantata.app/Contents/MacOS/Cantata"
'';

	meta = {
		description = "Graphical client for MPD";
		mainProgram = "cantata";
		homepage = "https://github.com/nullobsi/cantata";
		license = lib.licenses.gpl3Only;
		platforms = lib.platforms.unix;
	};
})
	


/*
		let
			name = "cantata";
			version = "3.5.0";
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
				kdePackages.kitemviews
				kdePackages.karchive
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
	*/
