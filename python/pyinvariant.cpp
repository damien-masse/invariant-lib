#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "ibex/ibex_IntervalVector.h"
#include "ibex/ibex_Function.h"
#include "ibex/ibex_Sep.h"

#include "smartSubPaving.h"
#include "domain.h"
#include "maze.h"
#include "sepmaze.h"
#include "dynamics.h"
#include "dynamics_function.h"
#include "vibesMaze.h"
#include <string>

namespace py = pybind11;
using namespace pybind11::literals;

using namespace invariant;
using namespace ibex;

 PYBIND11_MODULE(pyinvariant, m){
   m.doc() = "Python binding of invariant-lib";

   // ********* ENUM *********
   py::enum_<invariant::Domain<>::DOMAIN_INITIALIZATION>(m, "DOMAIN_INITIALIZATION")
       .value("FULL_WALL", invariant::Domain<>::DOMAIN_INITIALIZATION::FULL_WALL)
       .value("FULL_DOOR", invariant::Domain<>::DOMAIN_INITIALIZATION::FULL_DOOR)
       .export_values();

   py::enum_<invariant::Dynamics::DYNAMICS_SENS>(m, "DYNAMICS_SENS")
       .value("FWD", invariant::Dynamics::DYNAMICS_SENS::FWD)
       .value("BWD", invariant::Dynamics::DYNAMICS_SENS::BWD)
       .value("FWD_BWD", invariant::Dynamics::DYNAMICS_SENS::FWD_BWD)
       .export_values();

  // ********* Graphs *********
  py::class_<invariant::SmartSubPaving<>>(m, "SmartSubPaving")
          .def(py::init<const IntervalVector&>(), "IntervalVector"_a)
          .def("bisect", &invariant::SmartSubPaving<>::bisect)
          .def("size", &invariant::SmartSubPaving<>::size)
  ;  

  // ********* Domain *********
  py::class_<invariant::Domain<>>(m, "Domain")
          .def(py::init<invariant::SmartSubPaving<>*, invariant::Domain<>::DOMAIN_INITIALIZATION>(),
               "paving"_a,
               "DOMAIN_INITIALIZATION"_a)
          .def("set_border_path_in", &invariant::Domain<>::set_border_path_in)
          .def("set_border_path_out", &invariant::Domain<>::set_border_path_out)
          .def("set_sep", &invariant::Domain<>::set_sep)
          .def("set_sep_input", &invariant::Domain<>::set_sep_input)
          .def("set_sep_output", &invariant::Domain<>::set_sep_output)
          .def("add_maze_union", (void (invariant::Domain<>::*)(std::vector<invariant::Maze<> *> maze_list)) &invariant::Domain<>::add_maze_union)
          .def("add_maze_union", (void (invariant::Domain<>::*)(invariant::Maze<> *maze)) &invariant::Domain<>::add_maze_union)
          .def("add_maze_inter", (void (invariant::Domain<>::*)(std::vector<invariant::Maze<> *> maze_list)) &invariant::Domain<>::add_maze_inter)
          .def("add_maze_inter", (void (invariant::Domain<>::*)(invariant::Maze<> *maze)) &invariant::Domain<>::add_maze_inter)
    ;

  // ********* Dynamics Function *********
  py::class_<invariant::Dynamics> dynamics(m, "Dynamics")
  ;

  py::class_<invariant::Dynamics_Function>(m, "DynamicsFunction", dynamics)
          .def(py::init<ibex::Function*, invariant::Dynamics::DYNAMICS_SENS>(),
               "f"_a,
               "DYNAMICS_SENS"_a)
          .def(py::init<std::vector<ibex::Function*>, invariant::Dynamics::DYNAMICS_SENS>(),
               "f_list"_a,
               "DYNAMICS_SENS"_a)
  ;

  // ********* Maze *********
  py::class_<invariant::Maze<>>(m, "Maze")
          .def(py::init<invariant::Domain<>*, invariant::Dynamics*>(),
               "domain"_a,
               "dynamics"_a)
          .def("contract", &invariant::Maze<>::contract)
          .def("init", &invariant::Maze<>::init)
    ;

  // SepMaze
  py::object sep = (py::object) py::module::import("pyibex").attr("Sep");
  py::class_<invariant::SepMaze<>>(m, "SepMaze", sep)
          .def(py::init<invariant::Maze<>*>(),"maze"_a)
          .def("separate", &invariant::SepMaze<>::separate)
  ;

  // ********* VibesMaze *********
  py::enum_<VibesMaze::VIBES_MAZE_TYPE>(m, "VIBES_MAZE_TYPE")
      .value("VIBES_MAZE_INNER", VibesMaze::VIBES_MAZE_INNER)
      .value("VIBES_MAZE_OUTER", VibesMaze::VIBES_MAZE_OUTER)
      .export_values()
  ;

  py::class_<VibesMaze>(m, "VibesMaze")
          .def(py::init<const std::string&,
               invariant::Maze<>*,
               VibesMaze::VIBES_MAZE_TYPE>(),
               "name"_a,
               "maze"_a,
               "VIBES_MAZE_TYPE"_a = VibesMaze::VIBES_MAZE_OUTER)
          .def(py::init<const std::string&,
               invariant::Maze<>*,
               invariant::Maze<>*>(),
               "name"_a,
               "maze_outer"_a,
               "maze_inner"_a)
          .def("setProperties", &VibesMaze::setProperties)
          .def("show", &VibesMaze::show)
          .def("drawCircle", &VibesMaze::drawCircle)
          .def("drawBox", (void (VibesMaze::*)(const ibex::IntervalVector &box, std::string params) const) &VibesMaze::drawBox)
          .def("drawBox", (void (VibesMaze::*)(double x_min, double x_max, double y_min, double y_max, std::string params) const) &VibesMaze::drawBox)
  ;

}
