package com.cit.uvccamera.app.event;

public class Event
{

    public static final int MSG_ON_SERVICE_CONNECTED = 1;
    public static final int MSG_ON_SERVICE_DISCONNECTED = 2;
    public static final int MSG_ON_SELECTED = 3;
    public static final int MSG_ON_UNSELECTED = 4;
    public static final int MSG_ON_CONNECTED = 5;
    public static final int MSG_ON_DISCONNECTED = 6;
    public static final int MSG_ON_FORMAT = 7;

    public int what;
    public Object obj;

    public Event(int what)
    {
        this.what = what;
    }
}
