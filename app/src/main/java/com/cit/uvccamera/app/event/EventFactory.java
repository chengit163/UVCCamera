package com.cit.uvccamera.app.event;

public class EventFactory
{
    public static Event getEvent(int what)
    {
        Event event = null;
        switch (what)
        {
            case Event.MSG_ON_SERVICE_CONNECTED:
                event = new Event(Event.MSG_ON_SERVICE_CONNECTED);
                break;
            case Event.MSG_ON_SERVICE_DISCONNECTED:
                event = new Event(Event.MSG_ON_SERVICE_DISCONNECTED);
                break;
            case Event.MSG_ON_SELECTED:
                event = new Event(Event.MSG_ON_SELECTED);
                break;
            case Event.MSG_ON_UNSELECTED:
                event = new Event(Event.MSG_ON_UNSELECTED);
                break;
            case Event.MSG_ON_CONNECTED:
                event = new Event(Event.MSG_ON_CONNECTED);
                break;
            case Event.MSG_ON_DISCONNECTED:
                event = new Event(Event.MSG_ON_DISCONNECTED);
                break;
            case Event.MSG_ON_FORMAT:
                event = new Event(Event.MSG_ON_FORMAT);
                break;
        }
        return event;
    }
}
