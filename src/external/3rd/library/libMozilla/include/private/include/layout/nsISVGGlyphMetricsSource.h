/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/layout/svg/renderer/public/nsISVGGlyphMetricsSource.idl
 */

#ifndef __gen_nsISVGGlyphMetricsSource_h__
#define __gen_nsISVGGlyphMetricsSource_h__


#ifndef __gen_nsISVGGeometrySource_h__
#include "nsISVGGeometrySource.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
#include "nsFont.h"

/* starting interface:    nsISVGGlyphMetricsSource */
#define NS_ISVGGLYPHMETRICSSOURCE_IID_STR "8e96a973-c8bb-43a2-8584-4976e75ca478"

#define NS_ISVGGLYPHMETRICSSOURCE_IID \
  {0x8e96a973, 0xc8bb, 0x43a2, \
    { 0x85, 0x84, 0x49, 0x76, 0xe7, 0x5c, 0xa4, 0x78 }}

/**
 * \addtogroup rendering_backend_interfaces Rendering Backend Interfaces
 * @{
 */
/**
 * Abstracts a description of a 'composite glyph' (character string
 * with associated font and styling information) in the SVG rendering
 * backend for use by a rendering engine's nsISVGRendererGlyphMetrics
 * objects.
 *
 * An engine-native glyph metrics object will be informed of changes
 * in its associated composite glyph with a call to its
 * nsISVGRendererGlyphMetrics::update() method with an OR-ed
 * combination of the UPDATEMASK_* constants defined in this interface
 * (and its base-interface).
 *
 * @nosubgrouping
 */
class NS_NO_VTABLE nsISVGGlyphMetricsSource : public nsISVGGeometrySource {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISVGGLYPHMETRICSSOURCE_IID)

  /**
   * @name Font
   * @{
   */
  /* [noscript] readonly attribute nsFont font; */
  NS_IMETHOD GetFont(nsFont *aFont) = 0;

  enum { UPDATEMASK_FONT = 32768U };

  /** @} */
/**
   * @name Character data
   * @{
   */
  /* readonly attribute DOMString characterData; */
  NS_IMETHOD GetCharacterData(nsAString & aCharacterData) = 0;

  enum { UPDATEMASK_CHARACTER_DATA = 65536U };

  /** @} */
/**
   * @name Text rendering mode
   * @{
   */
  enum { TEXT_RENDERING_AUTO = 0U };

  enum { TEXT_RENDERING_OPTIMIZESPEED = 1U };

  enum { TEXT_RENDERING_OPTIMIZELEGIBILITY = 2U };

  enum { TEXT_RENDERING_GEOMETRICPRECISION = 3U };

  /* readonly attribute unsigned short textRendering; */
  NS_IMETHOD GetTextRendering(PRUint16 *aTextRendering) = 0;

  enum { UPDATEMASK_TEXT_RENDERING = 131072U };

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISVGGLYPHMETRICSSOURCE \
  NS_IMETHOD GetFont(nsFont *aFont); \
  NS_IMETHOD GetCharacterData(nsAString & aCharacterData); \
  NS_IMETHOD GetTextRendering(PRUint16 *aTextRendering); \

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISVGGLYPHMETRICSSOURCE(_to) \
  NS_IMETHOD GetFont(nsFont *aFont) { return _to GetFont(aFont); } \
  NS_IMETHOD GetCharacterData(nsAString & aCharacterData) { return _to GetCharacterData(aCharacterData); } \
  NS_IMETHOD GetTextRendering(PRUint16 *aTextRendering) { return _to GetTextRendering(aTextRendering); } \

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISVGGLYPHMETRICSSOURCE(_to) \
  NS_IMETHOD GetFont(nsFont *aFont) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFont(aFont); } \
  NS_IMETHOD GetCharacterData(nsAString & aCharacterData) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCharacterData(aCharacterData); } \
  NS_IMETHOD GetTextRendering(PRUint16 *aTextRendering) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetTextRendering(aTextRendering); } \

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSVGGlyphMetricsSource : public nsISVGGlyphMetricsSource
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISVGGLYPHMETRICSSOURCE

  nsSVGGlyphMetricsSource();

private:
  ~nsSVGGlyphMetricsSource();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSVGGlyphMetricsSource, nsISVGGlyphMetricsSource)

nsSVGGlyphMetricsSource::nsSVGGlyphMetricsSource()
{
  /* member initializers and constructor code */
}

nsSVGGlyphMetricsSource::~nsSVGGlyphMetricsSource()
{
  /* destructor code */
}

/* [noscript] readonly attribute nsFont font; */
NS_IMETHODIMP nsSVGGlyphMetricsSource::GetFont(nsFont *aFont)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString characterData; */
NS_IMETHODIMP nsSVGGlyphMetricsSource::GetCharacterData(nsAString & aCharacterData)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned short textRendering; */
NS_IMETHODIMP nsSVGGlyphMetricsSource::GetTextRendering(PRUint16 *aTextRendering)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsISVGGlyphMetricsSource_h__ */
