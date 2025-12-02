<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop-application">
  <id>@PROJECT_REV_ID@</id>
  
  <name>Cantata</name>
  <summary>Control your music server</summary>
  
  <metadata_license>CC-BY-SA-4.0</metadata_license>
  <project_license>GPL-3.0-or-later</project_license>
  
  <description>
    <p>
        A graphical MPD (Music Player Daemon) client with support for
        tagging, ReplayGain tag creation,
        queue management, and online features.
    </p>
	<p>
		This software connects to a running MPD server on a local or
		remote machine. Check out their project page for more information.
	</p>
	<ul>
		<li>Manage your playlists with an advanced interface.</li>
		<li>Flip through your music library with ease.</li>
		<li>Read information on your favorite artists.</li>
		<li>Automatically find lyrics to your songs.</li>
	</ul>
  </description>

  <developer id="dog.unix.www.nullobsi">
  	<name>Citlali del Rey</name>
  </developer>
  
  <launchable type="desktop-id">@PROJECT_REV_ID@.desktop</launchable>
  <branding>
    <color type="primary" scheme_preference="light">#96b6ee</color>
    <color type="primary" scheme_preference="dark">#335371</color>
  </branding>

  <content_rating type="oars-1.1" />

  <url type="homepage">https://cantata.unix.dog/</url>
  <url type="vcs-browser">https://cantata.unix.dog/src</url>
  <screenshots>
    <screenshot type="default">
	  <image>https://raw.githubusercontent.com/nullobsi/cantata/3198a3362c3d5d7425eea4f50043ba3198b75723/screenshots/mainwindow.png</image>
	  <caption>A playlist is open on the right, with some albums to add on the left.</caption>
	</screenshot>
    <screenshot>
	  <image>https://raw.githubusercontent.com/nullobsi/cantata/3198a3362c3d5d7425eea4f50043ba3198b75723/screenshots/artist1.png</image>
	  <caption>The information pane is open displaying an artist's biography and the current song lyrics.</caption>
	</screenshot>
    <screenshot>
	  <image>https://raw.githubusercontent.com/nullobsi/cantata/3198a3362c3d5d7425eea4f50043ba3198b75723/screenshots/artist2.png</image>
	  <caption>The information pane is open, showing related songs and albums.</caption>
	</screenshot>
  </screenshots>

  <releases>
	<release version="3.3.1" date="2025-04-24">
		<description>
			<p>Patch release.</p>
			<ul>
				<li>Use tags from filename if missing</li>
				<li>Fix cdparanoia</li>
				<li>Link to correct musicbrainz library</li>
				<li>Update flatpak workflow</li>
				<li>Use latest version of KDE platform</li>
				<li>Add nix flake</li>
			</ul>
		</description>
	</release>
    <release version="3.3.0" date="2024-11-13">
	  <description>
	    <p>Minor release.</p>
		<ul>
		  <li>Migrate data from older Cantata versions.</li>
		  <li>Fix libVLC build.</li>
		  <li>Fix MPRIS desktopEntry property.</li>
		  <li>Fix deprecated FFMPEG function call.</li>
		  <li>Fix podcast saving and date retrieval.</li>
		  <li>Use embedded cover art from MPD server when needed.</li>
		</ul>
	  </description>
	</release>
    <release version="3.2.1" date="2024-08-06">
	  <description>
	    <p>Patch release.</p>
		<ul>
			<li>Fix now-playing notification being persistent.</li>
			<li>Respect environment settings for showing icons in app menus.</li>
			<li>Improve GUI responsiveness when changing tracks &amp; now-playing notifications are enabled.</li>
			<li>Notifications when running in Flatpak now update properly.</li>
			<li>Fix in-app HTTP stream playback.</li>
		</ul>
	  </description>
	</release>
    <release version="3.2.0" date="2024-05-30">
	  <description>
		<p>Minor release.</p>
		<ul>
			<li>Fix menu options having incorrect keybindings.</li>
			<li>Fix radio stream favorites button not working on search.</li>
			<li>Enable menu icons on macOS.</li>
			<li>Update icons to FontAwesome 6.</li>
			<li>Improve palette change handling.</li>
		</ul>
	  </description>
	</release>
  	<release version="3.1.0" date="2024-05-25">
	  <description>
	    <p>Bugfix release.</p>
		<ul>
		  <li>Fix crash when enabling notifications.</li>
		  <li>Fix Qt warnings.</li>
		  <li>Fix notifications on Flatpak.</li>
		  <li>Fix tray icon image not showing up.</li>
		</ul>
	  </description>
	</release>
    <release version="3.0.2" date="2024-05-22">
	  <description>
	    <p>Initial release.</p>
		<ul>
		  <li>Build with Qt6.</li>
		  <li>Overhaul build system.</li>
		  <li>Make some executable search behavior more consistent.</li>
		</ul>
	  </description>
	</release>
  </releases>

  <translation type="qt">cantata</translation>

  <requires>
    <control>keyboard</control>
    <control>pointing</control>
  </requires>
</component>
