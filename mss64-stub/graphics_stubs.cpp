// Stub for Graphics::getUiCanvasWidth/getUiCanvasHeight needed by the
// fresh swgClientUserInterface.lib but missing from the prebuilt
// (older) clientGraphics.lib. Returns sane defaults equal to the
// current frame buffer size; the GodClient never depends on canvas
// scaling for its Qt-hosted GameWidget.

class Graphics
{
public:
	static int getUiCanvasWidth();
	static int getUiCanvasHeight();
	static int getFrameBufferMaxWidth();
	static int getFrameBufferMaxHeight();
};

int Graphics::getUiCanvasWidth()
{
	return Graphics::getFrameBufferMaxWidth();
}

int Graphics::getUiCanvasHeight()
{
	return Graphics::getFrameBufferMaxHeight();
}
