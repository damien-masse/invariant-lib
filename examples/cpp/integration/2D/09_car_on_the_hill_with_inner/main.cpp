#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"
#include "vibesMaze.h"

#include "ibex_SepFwdBwd.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include "graphiz_graph.h"
#include <omp.h>

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x1, x2;

    IntervalVector space(2);
    space[0] = ibex::Interval(-2, 13.0);
    space[1] = ibex::Interval(-3, 5);

    invariant::SmartSubPaving<> paving(space);

    // ****** Domain Outer ******* //
    invariant::Domain<> dom_outer(&paving, FULL_WALL);

    double x1_c, x2_c, r;
//    x1_c = 3.0;
//    x2_c = 2.0;
    x1_c = 0.0;
    x2_c = 0.0;
    r = 1.0;

    IntervalVector box_init(2);
    box_init[0] = ibex::Interval(-1, 1);
    box_init[1] = ibex::Interval(-1, 1);

    Function f_sep_outer(x1, x2, pow(x1-x1_c, 2)+pow(x2-x2_c, 2)-pow(r, 2));
    SepFwdBwd s_outer(f_sep_outer, LT); // LT, LEQ, EQ, GEQ, GT
    dom_outer.set_sep_input(&s_outer);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    // ****** Domain Inner ******* //
    invariant::Domain<> dom_inner(&paving, FULL_DOOR);

    Function f_sep_inner(x1, x2, pow(x1-x1_c, 2)+pow(x2-x2_c, 2)-pow(r, 2));
    SepFwdBwd s_inner(f_sep_inner, GEQ); // LT, LEQ, EQ, GEQ, GT
    dom_inner.set_sep(&s_inner);

    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);

    // ****** Dynamics Outer & Inner ******* //
//    ibex::Interval u(1.2);
//    ibex::Interval u(-1.2);
    ibex::Interval u(-1.2, 1.2);
    ibex::Function f(x1, x2, Return(x2,
                                     -9.81*sin((1.1*sin(1.2*x1)-1.2*sin(1.1*x1))/2.0)-0.7*x2+u));
    Dynamics_Function dyn(&f, FWD);

    // ******* Mazes ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn);
    invariant::Maze<> maze_inner(&dom_inner, &dyn);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    
    for(int i=0; i<16; i++){
        cout << i  << endl;
        paving.bisect();
        maze_outer.contract();
        maze_inner.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    VibesMaze v_maze("SmartSubPaving", &maze_outer, &maze_inner);
//    VibesMaze v_maze("SmartSubPaving", &maze_inner);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.show();

    v_maze.drawCircle(0.0, 0.0, 1, "black[red]");

//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(-1.2, -1.16);
//    position_info[1] = ibex::Interval(3.62);
//    v_maze.get_room_info(&maze_inner, position_info);
//    v_maze.show_room_info(&maze_inner, position_info);


    vibes::endDrawing();

}
