/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      data.h                                                      //
//                                                                         //
//  Purpose:   Header file for search data classes                         //
//                                                                         //
//  Author(s): Sysoyev A., Barkalov K., Sovrasov V.                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __SEARC_DATA_ITERATOR_H__
#define __SEARC_DATA_ITERATOR_H__

#include "SearchData.h"
#include "TreeNode.h"



// ------------------------------------------------------------------------------------------------
class TSearcDataIterator
{
  friend class TSearchData;
protected:
  TSearchData *pContainer;
  TTreeNode *pObject;

public:
  TSearcDataIterator();

  TSearcDataIterator& operator++();
  TSearcDataIterator operator++(int);
  TSearcDataIterator& operator--();
  TSearcDataIterator operator--(int);
  operator void*() const;
  TSearchInterval* operator->();
  TSearchInterval* operator *() const;
};



#endif
// - end of file ----------------------------------------------------------------------------------
