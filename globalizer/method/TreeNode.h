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

#ifndef __TREE_NODE_H__
#define __TREE_NODE_H__


#include "SearchInterval.h"
#include "SearchIntervalFactory.h"



class TSearchInterval;
// ------------------------------------------------------------------------------------------------
struct TTreeNode
{
  TSearchInterval *pInterval;
  unsigned char Height;
  TTreeNode *pLeft;
  TTreeNode *pRight;
  TTreeNode *pParent;
  TTreeNode(TSearchInterval &p)
  {
    pInterval = TSearchIntervalFactory::CreateSearchInterval(p);
      //new TSearchInterval(p);
    Height = 1;
    pParent = pLeft = pRight = NULL;
  }
  ~TTreeNode() { delete pInterval; }
};


#endif
// - end of file ----------------------------------------------------------------------------------
