set CODESIGN_OPTIONS -f --timestamp --verbose=4 --options=runtime -i dog.unix.cantata.Cantata

echo "Signing frameworks..."
for FRAMEWORK in (find $argv[1] -name "*.framework")
	echo "About to sign" (basename $argv[1])
	codesign $CODESIGN_OPTIONS --sign $DEVELOPER_ID $FRAMEWORK
end

echo "Signing the dylibs..."
find $argv[1]/Contents/Frameworks -name "*.dylib" -exec codesign $CODESIGN_OPTIONS --sign $DEVELOPER_ID '{}' ';'

echo "Signing the plugins..."
find $argv[1]/Contents/PlugIns -name "*.dylib" -exec codesign $CODESIGN_OPTIONS --sign $DEVELOPER_ID '{}' ';'

echo "Signing the other executables..."
codesign $CODESIGN_OPTIONS --sign $DEVELOPER_ID $argv[1]/Contents/MacOS/cantata-tags
codesign $CODESIGN_OPTIONS --sign $DEVELOPER_ID $argv[1]/Contents/MacOS/cantata-replaygain

echo "Signing the bundle..."
codesign $CODESIGN_OPTIONS --sign $DEVELOPER_ID $argv[1]


echo "Sending notarization request to Apple..."
/usr/bin/ditto -c -k --keepParent $argv[1] $argv[1].zip
xcrun notarytool submit --apple-id $APPLE_ID --team-id $TEAM_ID --keychain-profile $KEYCHAIN_PROFILE --wait $argv[1].zip
xcrun stapler staple $argv[1]
