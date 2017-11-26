#ifndef GRAPHIZ_GRAPH_H
#define GRAPHIZ_GRAPH_H

#include "smartSubPaving.h"
#include <string>
#include "ibex/ibex_IntervalVector.h"

class Graphiz_Graph
{
public:
    Graphiz_Graph(const std::string &file_name, invariant::SmartSubPaving<ibex::IntervalVector> *g);
};

#endif // GRAPHIZ_GRAPH_H
