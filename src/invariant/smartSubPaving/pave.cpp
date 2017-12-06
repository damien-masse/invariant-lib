#include "pave.h"
#include "room.h"

namespace invariant{

/// ******************  ibex::IntervalVector ****************** ///

template<>
int Pave<ibex::IntervalVector, std::vector<ibex::IntervalVector>>::get_dim_inter_boundary(const ibex::IntervalVector &box){
    IntervalVector intersection(get_dim(), ibex::Interval::EMPTY_SET);
    for(invariant::Face<ibex::IntervalVector, std::vector<ibex::IntervalVector>> *f:m_faces_vector){
        intersection |= (box & f->get_position());
    }
    return m_dim - invariant::get_nb_dim_flat<ibex::IntervalVector, std::vector<ibex::IntervalVector>>(intersection);
}

template <>
const ibex::IntervalVector Pave<ibex::IntervalVector, std::vector<ibex::IntervalVector>>::get_position_typed() const{
    return m_position;
}

/// ******************  ppl::C_Polyhedron ****************** ///

template <>
const ppl::C_Polyhedron Pave<ppl::C_Polyhedron, std::vector<ppl::Generator_System>>::get_position_typed() const{
    return iv_2_polyhedron(this->m_position);
}

}
