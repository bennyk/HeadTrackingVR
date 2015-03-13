

#include <EGL/egl.h> // requires ndk r5 or newer
//#include <GLES/gl.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

struct NullApp : public MyGlApp{

	NullApp() : MyGlApp() {}
	~NullApp() {}

	virtual bool initialize(){
		return MyGlApp::initialize();
	}

	virtual void onDraw() {
	    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		return;
	}

};
