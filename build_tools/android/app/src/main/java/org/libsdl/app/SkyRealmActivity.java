package org.libsdl.app;

import android.app.Activity;
import android.os.Bundle;

public class SkyRealmActivity extends SDLActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    protected String[] getLibraries() {
        return new String[] {
            "SDL2",
            "SDL2_ttf",
            "SDL2_mixer",
            "skyrealm_jni"
        };
    }
}