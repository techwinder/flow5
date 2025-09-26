/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include <xflgraph/graph/graph.h>
class GraphWt;

void exportGraphToSvg(const GraphWt *pGraphWt, Graph const *pGraph, QString &tempfilepath);
void exportGraphDataToFile(Graph const*pGraph);
