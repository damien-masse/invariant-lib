#include "ibex/ibex_SepFwdBwd.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"
#include "vibesMaze.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include "graphiz_graph.h"
#include <omp.h>
#include "vtk_graph.h"

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x, y, z;

    IntervalVector space(3);
    space[0] = Interval(-10, 10);
    space[1] = Interval(-10, 10);
    space[2] = Interval(-10, 10);

    // ****** Domain ******* //
    SmartSubPaving paving(space);
    invariant::Domain dom(&paving, FULL_DOOR);

    Function f_sep(x, y, z, pow(x, 2)+pow(y, 2)+pow(z, 2)-pow(0.5, 2));
    SepFwdBwd s(f_sep, GEQ); // LT, LEQ, EQ, GEQ, GT
    dom.set_sep(&s);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    // ****** Dynamics ******* //
    ibex::Function f(x, y, z, Return(x-(x+y)*(pow(x,2)+pow(y,2)),
                                    y+(x-y)*(pow(x,2)+pow(y,2)),
                                     -z*(1+pow(x,2)+pow(y,2))));
    Dynamics_Function dyn(&f, FWD_BWD);

    // ******* Maze ********* //
    Maze maze(&dom, &dyn);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    for(int i=0; i<28; i++){
        cout << "-----" << i << "-----" << endl;
        paving.bisect();
        cout << "nb contractions = " << maze.contract() << " - ";
        cout << "paving size = " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    Vtk_Graph vtk_graph("torus", &paving, false);
    vtk_graph.show_graph();
    vtk_graph.show_maze(&maze);

//    IntervalVector position_info(3);
//    position_info[0] = Interval(0.5);
//    position_info[1] = Interval(0.5);
//    position_info[2] = Interval(0.2);
//    vtk_graph.show_room_info(&maze, position_info);

}
