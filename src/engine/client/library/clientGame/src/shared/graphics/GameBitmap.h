//===================================================================
//
// GameBitmap.h
// asommers
//
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_GameBitmap_H
#define INCLUDED_GameBitmap_H

//===================================================================

class Shader;

//===================================================================

class GameBitmap
{
public:

	GameBitmap (const Shader* shader, float width, float height);
	~GameBitmap ();

	float         getWidth () const;
	float         getHeight () const;
	const Shader* getShader () const;

	void          render (float x, float y, bool centerOnPoint=false) const;

private:

	const Shader* m_shader;
	float         m_width;
	float         m_height;
};

//===================================================================

#endif

