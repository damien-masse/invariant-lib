#include "graph.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include <omp.h>

#include "vtk_graph.h"

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x1, x2, x3;

    IntervalVector space(3);
    space[0] = Interval(-3.5,3.5);
    space[1] = Interval(-4.5,4.5);
    space[2] = Interval(0,10);

    Graph graph(space);

    // ****** Domain Outer ******* //
    invariant::Domain dom_outer(&graph);

    double x1_c, x2_c, x3_c, r;
    x1_c = -0.27;
    x2_c = 1.9;
    x3_c = 0;
    r = 0.1;

    Function f_sep_outer(x1, x2, x3, pow(x1-x1_c, 2)+pow(x2-x2_c, 2) + pow(x3-x3_c, 2) - pow(r, 2));
    SepFwdBwd s_outer(f_sep_outer, LT); // LT, LEQ, EQ, GEQ, GT
    dom_outer.set_sep(&s_outer);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    // ****** Domain Inner ******* //
    invariant::Domain dom_inner(&graph);

    SepFwdBwd s_inner(f_sep_outer, GEQ); // LT, LEQ, EQ, GEQ, GT
    dom_inner.set_sep(&s_inner);

    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);

    // ****** Dynamics Outer & Inner ******* //
        ibex::Function f(x1, x2, x3, Return(x2,
                                        (1.0*(1.0-pow(x1, 2))*x2-x1),
                                            2.0+0*x3));
//    ibex::Function f(x1, x2, Return(x2,
//                                    (1.0*(1.0-pow(x1, 2))*x2-x1)+Interval(-0.3, 0.3)));
    Dynamics_Function dyn_outer(&f);
    Dynamics_Function dyn_inner(&f);

    // ******* Mazes ********* //
    Maze maze_outer(&dom_outer, &dyn_outer, MAZE_FWD, MAZE_PROPAGATOR);
    Maze maze_inner(&dom_inner, &dyn_inner, MAZE_FWD, MAZE_CONTRACTOR);

    Vtk_Graph vtk_graph("vdp_3D", &graph);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    maze_outer.contract();
    for(int i=0; i<20; i++){
        graph.bisect();
        cout << i << " inner - " << maze_inner.contract() << " - " << graph.size() << endl;
        cout << i << " outer - " << maze_outer.contract() << " - " << graph.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << graph << endl;

    vtk_graph.show_graph();
    vtk_graph.show_maze(&maze_outer, "outer");
//    vtk_graph.show_maze(&maze_inner, "inner");

}