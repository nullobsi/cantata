<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop-application">
  <id>@PROJECT_REV_ID@</id>
  
  <name>Cantata</name>
  <summary>Listen to and manage your Music Player Daemon (MPD) library</summary>
  
  <metadata_license>CC-BY-SA-4.0</metadata_license>
  <project_license>GPL-3.0-or-later</project_license>
  
  <description>
    <p>
		A graphical MPD client with support for tagging, ReplayGain tag creation,
		queue management, and online features.
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
    <color type="primary" scheme_preference="light">#1c71d8</color>
    <color type="primary" scheme_preference="dark">#99c1f1</color>
  </branding>

  <content_rating type="oars-1.1" />

  <url type="homepage">https://cantata.unix.dog/</url>
  <url type="vcs-browser">https://cantata.unix.dog/src</url>

  <screenshots>
    <screenshot type="default">
	  <image>https://raw.githubusercontent.com/nullobsi/cantata/5e991803b8e4b4058419c647f2cfcd3e81273bde/screenshots/mainwindow.png</image>
	  <caption>A playlist is open on the right, with some albums to add on the left.</caption>
	</screenshot>
    <screenshot>
	  <image>https://raw.githubusercontent.com/nullobsi/cantata/5e991803b8e4b4058419c647f2cfcd3e81273bde/screenshots/artist1.png</image>
	  <caption>The information pane is open displaying an artist's biography and the current song lyrics.</caption>
	</screenshot>
    <screenshot>
	  <image>https://raw.githubusercontent.com/nullobsi/cantata/5e991803b8e4b4058419c647f2cfcd3e81273bde/screenshots/artist2.png</image>
	  <caption>The information pane is open, showing related songs and albums.</caption>
	</screenshot>
  </screenshots>

  <releases>
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
</component>
