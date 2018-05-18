#include "ofApp.h"

int main()
{
#ifdef TARGET_RASPBERRY_PI
  ofGLESWindowSettings settings;
  settings.windowMode = OF_FULLSCREEN;
#else
  ofGLFWWindowSettings settings;
#endif
  auto window = ofCreateWindow(settings);
  auto app = make_shared<ofApp>();
  ofRunApp(window, app);

  return ofRunMainLoop();
}
