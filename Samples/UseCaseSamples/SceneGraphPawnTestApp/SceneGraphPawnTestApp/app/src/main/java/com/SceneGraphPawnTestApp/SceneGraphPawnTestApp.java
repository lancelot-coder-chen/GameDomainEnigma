
package com.SceneGraphPawnTestApp;
import android.os.Bundle;
import lib.bridge.enigma.ApplicationBridgeActivity;

public class SceneGraphPawnTestApp extends ApplicationBridgeActivity
{
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        initBridgeCallback();
        super.onCreate(savedInstanceState);
        initAdditionalMountPath();
    }
    public native void initBridgeCallback();
    public native void initAdditionalMountPath();
    static {
        System.loadLibrary("SceneGraphPawnTest_Jni");
    }
}
