// ICoreServiceCallback.aidl
package com.cit.uvccamera.aidl;

// Declare any non-default types here with import statements

interface ICoreServiceCallback
{
    oneway void onSelected(int id);

    oneway void onUnselected();

    oneway void onConnected();

    oneway void onDisConnected();

    oneway void onFormat(int pixelformat, int fps, int width, int height);
}
