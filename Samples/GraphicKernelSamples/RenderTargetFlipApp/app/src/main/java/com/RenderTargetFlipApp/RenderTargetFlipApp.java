
package com.RenderTargetFlipApp;

import android.os.Bundle;
import lib.bridge.enigma.ApplicationBridgeActivity;

public class RenderTargetFlipApp extends ApplicationBridgeActivity
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
    /* This is the static constructor used to load the
     * 'ParticlePrimitiveTest' library when the class is
     * loaded.
     */
    static {
        System.loadLibrary("RenderTargetFlipApp_Jni");
    }
}