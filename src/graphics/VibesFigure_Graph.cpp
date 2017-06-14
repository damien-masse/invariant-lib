#include "VibesFigure_Graph.h"
#include <ibex.h>
#include "vibes.h"

using namespace invariant;
using namespace ibex;
using namespace std;

VibesFigure_Graph::VibesFigure_Graph(const std::string& figure_name, Graph *g): VibesFigure(figure_name){
    m_graph = g;
    m_overhead_factor = 0.2; // 20%
}

void VibesFigure_Graph::show() const{
    IntervalVector bounding_box(m_graph->dim(), Interval::EMPTY_SET);
    vibes::newGroup("graph_bisectable", "gray[gray]", vibesParams("figure", m_name));
    vibes::newGroup("graph_not_bisectable", "lightGray[lightGray]", vibesParams("figure", m_name));


    vibes::Params params_bisectable, params_not_bisectable;
    params_bisectable = vibesParams("figure", m_name, "group", "graph_bisectable", "FaceColor","none","EdgeColor","gray");
    params_not_bisectable = vibesParams("figure", m_name, "group", "graph_not_bisectable", "FaceColor","none","EdgeColor","lightGray");

    for(Pave*p:m_graph->paves()){
        ibex::IntervalVector box(p->coordinates());
        vibes::drawBox(box, params_bisectable);
        bounding_box |= box;
    }
    for(Pave*p:m_graph->paves_not_bisectable()){
        vibes::drawBox(p->coordinates(), params_not_bisectable);
        bounding_box |= p->coordinates();
    }

    double overhead_x0 = bounding_box.diam()[0]*m_overhead_factor;
    double overhead_x1 = bounding_box.diam()[1]*m_overhead_factor;
    vibes::axisLimits(bounding_box[0].lb()-overhead_x0, bounding_box[0].ub()+overhead_x0,
                      bounding_box[1].lb()-overhead_x1, bounding_box[1].ub()+overhead_x1,
                      m_name);
}

