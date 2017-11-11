// \addtodoc

#ifndef INC_NXNITERATOR_H
#define INC_NXNITERATOR_H

/*! \class		CNxNIterator NxNIterator.h
 *
 *  \brief		CNxNIterator is a class defining an interface that can be used for all iterator
 *              based enumerations in the NxN Integrator SDK.
 *
 *              To have the possibility of enumerating all kind of informations in a thread-safe way, it
 *              is common to use iterator objects, in which the current state of enumeration is stored.<br><br>
 *
 *              The following example shows how iterators can be used when enumerating the children of
 *              a CNxNNode using GetFirstChild() and GetNextChild():<br><br>
 *	\code

    // the iterator for node enumeration. Its type is derived from CNxNIterator
    TNxNNodeIterator it;

    // get the first child
    CNxNNode* pCurrentNode = pRootNode->GetFirstChild(&it);

    // enumerate all children until the end has been reached
    while (!it.IsEnd())
    {
        // do something with the node
        ....

        // retrieve the next child node
        pCurrentNode = pRootNode->GetNextChild(&it);
    }

 *	\endcode
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	1.00
 *
 *  \date		2001
 *
 *	\mod
 *		[ap]-06-Jun-2001 file created.
 *	\endmod
 */

struct NXNINTEGRATORSDK_API CNxNIterator
{

    /*!	\fn			virtual bool IsEnd() const = 0;
     *	
     *				
     *	
     *	\param		none
     *	
     *	\return		none
     *	
     *	\note		
     */
    virtual bool IsEnd() const = 0;
};

#endif // INC_NXNITERATOR_H

