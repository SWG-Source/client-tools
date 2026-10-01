#ifndef INCLUDED_VideoBlit_H
#define INCLUDED_VideoBlit_H
class Video;
// Internal shared texture rendering; copy executes while the texture is locked.
namespace VideoBlit
{
	bool construct();
	void destroy();
	void doFrame(void *context, void (*copy)(void *, void *, int, unsigned));
	void draw(Video *video, int screenX, int screenY, int screenCX, int screenCY);
}
#endif
