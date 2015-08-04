// \addtodoc

#ifndef INC_NXN_ARRAY_H
#define INC_NXN_ARRAY_H

/*! \class      CNxNArray NxNArray.h
 *
 *  \brief      This is a general template array class similar to MFC CArray.
 *
 *  \author     Axel Pfeuffer, Helmut Klinger
 *
 *  \version    1.0
 *
 *  \date       2000
 *
 *  \mod
 *      [ap]-02-Oct-2000 file created.
 *      [gs]-11-Oct-2000 file renamed and integrated into SDK.
 *      [ap]-05-Jun-2001 Documentation to each method added if missing and rewritten.
 *  \endmod
 */

// necessary include for using STL's vector template
#include <vector>

#include "NxNSTLAllocator.h"


class CNxNOutOfMemoryException {};
class CNxNNotInitializedException {};

template <class TArrayItem> class CNxNArray
{
    private:
        typedef std::vector< TArrayItem, CNxNSTLAllocator<TArrayItem> > TArray;
        TArray* m_pArray;

    public:
        //---------------------------------------------------------------------------
        //  construction/destruction
        //---------------------------------------------------------------------------

        /*! \fn         inline CNxNArray::CNxNArray()
         *  
         *              Default constructor.
         *  
         *  \param      none
         *  
         *  \return     none
         *  
         *  \note       
         */
        inline CNxNArray() 
        {
            m_pArray = new TArray;
            if (m_pArray == NULL)
            {
                throw CNxNOutOfMemoryException();
            }
        };

        /*! \fn         inline CNxNArray::CNxNArray(const CNxNArray<TArrayItem>& srcArray)
         *  
         *              Copy constructor.
         *  
         *  \param      srcArray    array containing the data to copy
         *  
         *  \return     none
         *  
         *  \note       
         */
        inline CNxNArray(const CNxNArray<TArrayItem>& srcArray)
        {
            m_pArray = new TArray;
            if (m_pArray != NULL)
            {
                RemoveAllAndCopyFrom(srcArray);
            }
            else
            {
                throw CNxNOutOfMemoryException();
            }
        }
        

        /*! \fn         virtual ~CNxNArray() 
         *  
         *              Default destructor. Removes all entries if the array is not empty.
         *  
         *  \param      none
         *  
         *  \return     none
         *  
         *  \note       
         */
        virtual ~CNxNArray() 
        {
            // not empty? => clean up!
            if (!IsEmpty())
            {
                RemoveAll();
            }

            delete m_pArray;
            m_pArray = NULL;
        };

        //---------------------------------------------------------------------------
        // operators
        //---------------------------------------------------------------------------

        /*! \fn         inline const CNxNArray<TArrayItem>& operator =(const CNxNArray<TArrayItem>& srcArray)
         *  
         *              Assignment operator that copies the data from the source array.
         *              Previous data in the array will be automatically deleted.
         *  
         *  \param      srcArray        array containing the data to copy
         *  
         *  \return     none
         *  
         *  \note       
         */
        inline const CNxNArray<TArrayItem>& operator =(const CNxNArray<TArrayItem>& srcArray)
        {
            RemoveAllAndCopyFrom(srcArray);
            return *this;
        }



        /*! \fn         inline void RemoveAllAndCopyFrom(const CNxNArray<TArrayItem>& srcArray)
         *  
         *              Removes all elements from the array and copies the data
         *              from the specified array.
         *  
         *  \param      srcArray        array containing the data to copy
         *  
         *  \return     none
         *  
         *  \note       
         */
        inline void RemoveAllAndCopyFrom(const CNxNArray<TArrayItem>& srcArray)
        {
            RemoveAll();

            int nSize = srcArray.GetCount();

            for (int i = 0; i < nSize; i++)
            {
                Add(srcArray.GetAt(i));
            }
        }

        /*! \fn         inline bool Add(const TArrayItem& param)
         *  
         *              Adds an item to the end of the array.
         *  
         *  \param      param   item to add to the array
         *  
         *  \return     boolean value indicating if the element was sucessfully added
         *  
         *  \note       
         */
        inline bool Add(const TArrayItem& param)
        {
            // add parameter
            if (m_pArray == NULL)
            {
                throw CNxNNotInitializedException();
            }

            m_pArray->insert(m_pArray->end(), param);

            return true;
        };


        /*! \fn         inline TArrayItem GetAt(long nIndex) const
         *  
         *              Retrieves the item at the specified index position.
         *  
         *  \param      nIndex  long value containing the index position of the element to retrieve
         *  
         *  \return     TArrayItem value containing copy of the item at the specified position
         *  
         *  \note       
         */
        inline TArrayItem GetAt(long nIndex) const
        {
            if (m_pArray == NULL)
            {
                throw CNxNNotInitializedException();
            }

            return (*m_pArray)[nIndex];
        };


        /*! \fn         inline TArrayItem& ElementAt(long nIndex)
         *  
         *              Retrieves a reference to the item at the specified position. The reference
         *              allows you to directly manipulate the item in the array.
         *  
         *  \param      nIndex  long value containing the index position of the item to retrieve
         *  
         *  \return     reference to the TArrayItem value at the specified position
         *  
         *  \note       
         */
        inline TArrayItem& ElementAt(long nIndex)
        {
            if (m_pArray == NULL)
            {
                throw CNxNNotInitializedException();
            }

            return (*m_pArray)[nIndex];
        }

        /*! \fn         inline bool SetAt(long nIndex, const TArrayItem& param)
         *  
         *              Sets the item at the specified index position to the given value.
         *                                                          
         *  \param      nIndex  long value containing the index position of the item to set
         *  \param      param   item to add
         *  
         *  \return     boolean value indicating if the item could be set
         *  
         *  \note       
         */
        inline bool SetAt(long nIndex, const TArrayItem& param)
        {
            if (m_pArray == NULL)
            {
                throw CNxNNotInitializedException();
            }

            (*m_pArray)[nIndex] = param;

            return true;
        };

        /*! \fn         inline void RemoveAt(long nIndex)
         *  
         *              Removes an item entry at the specified position. This method will shrink the
         *              array. It is possible to remove array elements at arbitrary positions, which
         *              will change the index position of all following items.
         *              <br>Example:
         *              This is the array before the item is removed:<br><br>
                        <table border=0>
                            <tr><td><b>Index    </b></td><td><b>Value   </b></td>
                            <tr><td><i>0        </i></td><td><i>10      </i></td>
                            <tr><td><i>1        </i></td><td><i>50      </i></td>
                            <tr><td><i>2        </i></td><td><i>60      </i></td>
                            <tr><td><i>3        </i></td><td><i>100     </i></td>
                        </table><br><br>
                        Now the third element will be removed with RemoveAt(2). This is the resulting array content:<br><br>
                        <table border=0>
                            <tr><td><b>Index    </b></td><td><b>Value   </b></td>
                            <tr><td><i>0        </i></td><td><i>10      </i></td>
                            <tr><td><i>1        </i></td><td><i>50      </i></td>
                            <tr><td><i>2        </i></td><td><i>100     </i></td>
                        </table><br><br>
         *              
         *  
         *  \param      nIndex  long value containing the index position of the item to be removed
         *  
         *  \return     none
         *  
         *  \note       
         */
        inline void RemoveAt(long nIndex)
        {
            if (m_pArray == NULL)
            {
                throw CNxNNotInitializedException();
            }

            m_pArray->erase(m_pArray->begin() + nIndex);
        };


        /*! \fn         inline long GetCount() const
         *  
         *              Returns the number of elements contained in the array.
         *  
         *  \param      none
         *  
         *  \return     long value indicating the number of elements in the array
         *  
         *  \note       
         */
        inline long GetCount() const
        {
            if (m_pArray == NULL)
            {
                throw CNxNNotInitializedException();
            }

            return m_pArray->size();
        };

        /*! \fn         inline long GetSize() const
         *  
         *              Returns the size of the array.
         *  
         *  \param      none
         *  
         *  \return     long value indicating the size of the array
         *  
         *  \note       
         */
        inline long GetSize() const
        {
            return GetCount();
        }


        /*! \fn         inline bool IsEmpty() const
         *  
         *              Retrieves a boolean value indicating if the array is empty.
         *  
         *  \param      none
         *  
         *  \return     A boolean indicating if the array is empty.
         *  
         *  \note       
         */
        inline bool IsEmpty() const
        {
            return (GetSize() == 0);
        }


        /*! \fn         inline void Resize(long lNewSize)
         *  
         *              Resizes the array to the given size.
         *  
         *  \param      lNewSize    long containing the new size value
         *  
         *  \return     none
         *  
         *  \note       
         */
        inline void Resize(long lNewSize)
        {
            if (m_pArray == NULL)
            {
                throw CNxNNotInitializedException();
            }

            m_pArray->resize(lNewSize);
        }


        /*! \fn         inline void RemoveAll()
         *  
         *              Removes all item entries from the array. The result after a call to this method
         *              is an empty array.
         *  
         *  \param      none
         *  
         *  \return     none
         *  
         *  \note       
         */
        inline void RemoveAll()
        {
            if (m_pArray == NULL)
            {
                throw CNxNNotInitializedException();
            }

            m_pArray->clear();
        }


        /*! \fn         inline TArrayItem& operator [](int nIndex)
         *  
         *              Array index operator for direct element access.
         *  
         *  \param      nIndex  long value containing the array index position
         *  
         *  \return     reference to the TArrayItem value at the specified position
         *  
         *  \note       
         */
        inline TArrayItem& operator [](int nIndex)
        {
            return ElementAt(nIndex);
        }


        /*! \fn         inline TArrayItem operator [](int nIndex) const
         *  
         *              Array index operator for direct element access.
         *  
         *  \param      nIndex  long value containig the array index position
         *  
         *  \return     reference to the TArrayItem value at the specified position
         *  
         *  \note       
         */
        inline TArrayItem operator [](int nIndex) const
        {
            return GetAt(nIndex);
        }
};


#endif // INC_NXN_ARRAY_H
