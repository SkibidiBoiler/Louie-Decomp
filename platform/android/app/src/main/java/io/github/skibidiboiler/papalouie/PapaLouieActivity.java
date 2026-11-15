package io.github.skibidiboiler.papalouie;

import android.os.Build;
import android.os.Bundle;
import android.view.Display;
import android.view.WindowManager;

import org.libsdl.app.SDLActivity;

// SDLActivity loads libSDL3.so and libmain.so and calls SDL_main (our SDL3 callbacks in src/main.cpp).
public class PapaLouieActivity extends SDLActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestHighestRefreshRate();
    }

    // Many phones (e.g. 120 Hz panels) run apps at 60 Hz unless the window asks for a faster display mode.
    // The game logic does not depend on this: it always runs at the original frame rate.
    private void requestHighestRefreshRate() {
        Display display = Build.VERSION.SDK_INT >= Build.VERSION_CODES.R
                ? getDisplay() : getWindowManager().getDefaultDisplay();
        if (display == null) {
            return;
        }
        Display.Mode current = display.getMode();
        Display.Mode best = current;
        for (Display.Mode mode : display.getSupportedModes()) {
            if (mode.getPhysicalWidth() == current.getPhysicalWidth()
                    && mode.getPhysicalHeight() == current.getPhysicalHeight()
                    && mode.getRefreshRate() > best.getRefreshRate()) {
                best = mode;
            }
        }
        WindowManager.LayoutParams params = getWindow().getAttributes();
        params.preferredDisplayModeId = best.getModeId();
        getWindow().setAttributes(params);
    }
}
