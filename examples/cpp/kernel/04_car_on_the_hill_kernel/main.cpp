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

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x1, x2;

    IntervalVector space(2);
//    space[0] = ibex::Interval(-1.0, 13.0);
//    space[1] = ibex::Interval(-16, 16);
    space[0] = ibex::Interval(-1.0, 13.0);
    space[1] = ibex::Interval(-6, 6);

    invariant::SmartSubPaving<> paving(space);

    // ****** Domain Outer ******* //
    invariant::Domain<> dom_outer(&paving, FULL_DOOR);

    dom_outer.set_border_path_in(true);
    dom_outer.set_border_path_out(false);

    // ****** Domain Inner ******* //
    invariant::Domain<> dom_inner(&paving, FULL_WALL);

    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(false);

    // ****** Dynamics Outer ******* //
//    ibex::Function f_outer(x1, x2, Return(x2,
//                                     (-9.81*sin( (-1.1/1.2*sin(x1)-1.2*sin(1.1*x1))/2.0 ) -0.7*x2 + ibex::Interval(-0.5, 0.5))));
    ibex::Function f_outer(x1, x2, Return(x2,
                                     -9.81*sin((1.1*sin(1.2*x1)-1.2*sin(1.1*x1))/2.0)-0.7*x2+ibex::Interval(-3.0, 3.0)));

    Dynamics_Function dyn_outer(&f_outer, BWD);

    // ****** Dynamics Inner ******* //
//    ibex::Function f_inner1(x1, x2, Return(-x2,
//                                     -(-9.81*sin( (-1.1/1.2*sin(x1)-1.2*sin(1.1*x1))/2.0 ) -0.7*x2 + ibex::Interval(-0.5))));
//    ibex::Function f_inner2(x1, x2, Return(-x2,
//                                     -(-9.81*sin( (-1.1/1.2*sin(x1)-1.2*sin(1.1*x1))/2.0 ) -0.7*x2 + ibex::Interval(0.5))));
    ibex::Function f_inner1(x1, x2, -Return(x2,
                                     (-9.81*sin((1.1*sin(1.2*x1)-1.2*sin(1.1*x1))/2.0)-0.7*x2+ibex::Interval(-3.0))));
    ibex::Function f_inner2(x1, x2, -Return(x2,
                                     (-9.81*sin((1.1*sin(1.2*x1)-1.2*sin(1.1*x1))/2.0)-0.7*x2+ibex::Interval(3.0))));

    vector<Function *> f_list_inner;
    f_list_inner.push_back(&f_inner1);
    f_list_inner.push_back(&f_inner2);
    Dynamics_Function dyn_inner(f_list_inner, FWD);

    // ******* Mazes ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);
    invariant::Maze<> maze_inner(&dom_inner, &dyn_inner);

    // ******* Algorithm ********* //
    vibes::beginDrawing();
    double time_start = omp_get_wtime();
    maze_inner.contract();
    for(int i=0; i<10; i++){
        paving.bisect();
        cout << i << " inner - " << maze_inner.contract() << " - " << paving.size() << endl;
        cout << i << " outer - " << maze_outer.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    VibesMaze v_maze("graph_inner", /*&maze_outer,*/ &maze_inner);
    v_maze.setProperties(0, 0, 512, 512);
    v_maze.show();

    IntervalVector position_info(2);
    position_info[0] = ibex::Interval(6.6);
    position_info[1] = ibex::Interval(-2.4, -2.05);
    v_maze.setProperties(600, 0, 512, 512);
    v_maze.show_room_info(&maze_inner, position_info);

    vibes::endDrawing();
}
