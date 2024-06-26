<!DOCTYPE node PUBLIC "-//freedesktop//DTD D-BUS Object Introspection 1.0//EN"
"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd">

<node>
  <interface name="@PROJECT_REV_ID@">
    <method name="showError">
        <arg type="s" name="error" direction="in"/>
    </method>
    <method name="dynamicStatus">
        <arg type="s" name="error" direction="in"/>
    </method>
    <method name="load">
      <arg name="urls" type="as" direction="in"/>
    </method>
    <method name="setCollection">
        <arg type="s" name="name" direction="in"/>
    </method>
    <property name='listActions' type='as' access='read'/>
    <method name="triggerAction">
        <arg type="s" name="name" direction="in"/>
    </method>
  </interface>
</node>
