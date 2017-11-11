// ======================================================================
//
// Exporter.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_Exporter_H
#define INCLUDED_Exporter_H

// ======================================================================

class BlueprintWriter;
class TextureElement;

// ======================================================================
/**
 * This class encapsulates both the BlueprintWriter object and the
 * list mapping texture element nodes to BlueprintWriter texture
 * indices.
 *
 * This class is used entirely within the TextureBuilder application.
 */

class Exporter
{
public:

	Exporter();
	~Exporter();

	BlueprintWriter  &getWriter();

	int               getTextureIndex(const TextureElement &textureElement);
	void              setTextureIndex(const TextureElement &textureElement, int index);

	bool              writeToFile(const std::string &fullPathname);

private:

	typedef stdmap<const TextureElement*, int>::fwd  TextureIndexMap;

private:

	std::auto_ptr<BlueprintWriter> m_blueprintWriter;
	std::auto_ptr<TextureIndexMap> m_textureIndexMap;

private:
	// disabled
	Exporter(const Exporter&);
	Exporter &operator =(const Exporter&);
};

// ======================================================================

inline BlueprintWriter &Exporter::getWriter()
{
	return *m_blueprintWriter;
} //lint !e1762 // Member function 'Exporter::getWriter(void)' could be made const // Logical const.

// ======================================================================

#endif
