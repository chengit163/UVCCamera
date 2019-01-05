package com.cit.uvccamera;

import java.nio.ByteBuffer;

public interface IFrameCallback
{
    void onFrame(ByteBuffer buffer);
}
