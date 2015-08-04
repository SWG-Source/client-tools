//======================================================================
//
// SwgCuiAttachmentList.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiAttachmentList_H
#define INCLUDED_SwgCuiAttachmentList_H

//======================================================================

class AttachmentData;
class CuiIconManagerCallback;
class Object;
class OutOfBandBase;
class CuiIconManagerCallback;
class UIPage;
class UIWidget;

//----------------------------------------------------------------------

class SwgCuiAttachmentList
{
public:

	class Renderable
	{
	public:
		Object *         object;
		AttachmentData * attachmentData;

	public:
		Renderable ();
		~Renderable ();
		UIWidget *       fetchIcon           (bool doRegister = false, CuiIconManagerCallback * callback = SwgCuiAttachmentList::cms_defaultIconCallback) const;
		static void      releaseIcon         (UIWidget & widget);
		static void      releaseIconsForPage (UIPage & page);

	private:
		Renderable & operator= (const Renderable & rhs);
		Renderable (const Renderable & rhs);
	};

	typedef stdvector<Renderable *>::fwd             RenderableVector;
	typedef stdvector<AttachmentData *>::fwd         Attachments;

	                          SwgCuiAttachmentList ();
	                         ~SwgCuiAttachmentList ();
	void                      set                  (const Attachments & attachments);
	const RenderableVector &  get                  () const;
	const Renderable *        get                  (int index) const;
	Renderable *              get                  (int index);
	void                      clear                ();
	bool                      empty                () const;
	Renderable *              push_back            (const AttachmentData & ad);
	Renderable *              back                 ();

	void                      erase                (int index);
	int                       size                 () const;

	void                      constructOob         (Unicode::String & oob) const;

private:

	static CuiIconManagerCallback  * cms_defaultIconCallback;

private:


	RenderableVector * m_rv;

	static int                             ms_instanceCount;
};

//----------------------------------------------------------------------

inline const SwgCuiAttachmentList::RenderableVector & SwgCuiAttachmentList::get () const
{
	return *m_rv;
}

//======================================================================

#endif
