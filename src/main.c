#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>
// #include <GL/glext.h> for things like shader and VBO
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <gbm.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "videodata.h"
#include "miditoframe.h"
#include "render.h"

int main(int argc, char **argv) {

	// first check our args

 	if (argc != 3) return -1;

	// Setup file paths
	char *videoPath = argv[1];
	char *MIDIpath = argv[2];

	// ----------------- INITIATE EGL & GBM ---------------------

	// we will use GBM for allocating gpu mem buffers

	EGLBoolean asres; // boolean 4 assert

	int GBMfd = open("/dev/dri/renderD128", O_RDWR); // open gpu for gbm
	assert(GBMfd > 0);

	struct gbm_device *gbm = gbm_create_device(GBMfd);
	assert(gbm != NULL);

	// tell egl where we are displaying our "pixels", which is gbm for our case
	EGLDisplay egldisplay = eglGetPlatformDisplay(EGL_PLATFORM_GBM_MESA, gbm, NULL);
	assert(egldisplay != NULL);

	asres = eglInitialize(egldisplay, NULL, NULL);
	assert(asres);

	// detect if some necessary extensions are supported
	const char *eglExtensionString = eglQueryString(egldisplay, EGL_EXTENSIONS);
	// we need this extension for our context and give control to the GL version
	assert(strstr(eglExtensionString, "EGL_KHR_create_context") != NULL);
	// necessary 4 not linking our context to a EGLSurface (window)
	assert(strstr(eglExtensionString, "EGL_KHR_surfaceless_context") != NULL);

	// time to set configs and tell EGL what OpenGL version are we using!
	// we are doing this for eglChooseConfigs, it will return a EGLConfigs which
	// can be used for any function here that needs a frame buffer config
	static const EGLint configAttribs[] = {EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT, EGL_NONE};
	EGLConfig config;
	EGLint count;

	asres = eglChooseConfig(egldisplay, configAttribs, &config, 1, &count);
	assert(asres);

	asres = eglBindAPI(EGL_OPENGL_API);
	assert(asres);

	// finally we can create our context and use it here,
	// we need our attribs again, to specify which opengl version are we using
	static const EGLint attribs[] = {EGL_CONTEXT_MAJOR_VERSION,
                                   3,
                                   EGL_CONTEXT_MINOR_VERSION,
                                   3,
                                   EGL_CONTEXT_OPENGL_PROFILE_MASK,
                                   EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
                                   EGL_NONE};
	EGLContext context = eglCreateContext(egldisplay, config, EGL_NO_CONTEXT, attribs);
	assert(context != EGL_NO_CONTEXT);

	asres = eglMakeCurrent(egldisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context); // finally put context here
	assert(asres);

	// ---------------------- MAIN PROGRAM -----------------------

	VideoData vData = GetVideoData(videoPath);
	NotesList notes = GetMidiFrames(MIDIpath, vData);

	// we got everything now, lets render!

	Render(vData, notes);

	// ------------------------- FINISH --------------------------
	free(notes.framestime);

	eglMakeCurrent(egldisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(egldisplay, context);
	eglTerminate(egldisplay);
	gbm_device_destroy(gbm);
	close(GBMfd);

  return 0;
}
