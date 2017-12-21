#include "door.h"

namespace ppl=Parma_Polyhedra_Library;

namespace invariant{

/// ******************  ibex::IntervalVector ****************** ///

template <>
void Door<ibex::IntervalVector, ibex::IntervalVector>::set_empty_private_output(){
    m_output_private->set_empty();
}

template <>
void Door<ibex::IntervalVector, ibex::IntervalVector>::set_empty_private_input(){
    m_input_private->set_empty();
}

template <> Door<ibex::IntervalVector, ibex::IntervalVector>::Door(invariant::Face<ibex::IntervalVector, ibex::IntervalVector> *face, invariant::Room<ibex::IntervalVector, ibex::IntervalVector> *room):
    m_input_public(face->get_position()),
    m_output_public(face->get_position())
{
    m_input_private = new IntervalVector(face->get_position());
    m_output_private = new IntervalVector(face->get_position());

    m_face = face;
    m_room = room;
    omp_init_lock(&m_lock_read);
}

template <>
void Door<ibex::IntervalVector, ibex::IntervalVector>::set_input_private(const ibex::IntervalVector& iv_input){
    *m_input_private = iv_input;
}

template <>
void Door<ibex::IntervalVector, ibex::IntervalVector>::set_output_private(const ibex::IntervalVector& iv_output){
    *m_output_private = iv_output;
}

template <>
void Door<ibex::IntervalVector, ibex::IntervalVector>::synchronize(){
    omp_set_lock(&m_lock_read);
    //    if(*m_input_private != m_input_public)
    //        m_update_input++;
    //    if(*m_output_private != m_output_public)
    //        m_update_output++;

    m_input_public = *m_input_private;
    m_output_public = *m_output_private;
    omp_unset_lock(&m_lock_read);
}

/// ******************  ppl::C_Polyhedron ****************** ///

template <>
void Door<ppl::C_Polyhedron, ppl::Generator_System>::set_empty_private_output(){
    *m_output_private = ppl::C_Polyhedron(m_face->get_pave()->get_dim(), ppl::EMPTY);
}

template <>
void Door<ppl::C_Polyhedron, ppl::Generator_System>::set_empty_private_input(){
    *m_input_private = ppl::C_Polyhedron(m_face->get_pave()->get_dim(), ppl::EMPTY);
}

template <> Door<ppl::C_Polyhedron, ppl::Generator_System>::Door(invariant::Face<ppl::C_Polyhedron, ppl::Generator_System> *face, invariant::Room<ppl::C_Polyhedron, ppl::Generator_System> *room)
{
    m_input_public = iv_2_polyhedron(face->get_position());
    m_input_public.minimized_constraints();
    m_output_public = m_input_public;
    m_input_private = new C_Polyhedron(m_input_public);
    m_output_private = new C_Polyhedron(m_input_public);

    m_face = face;
    m_room = room;
    omp_init_lock(&m_lock_read);
}

template <>
void Door<ppl::C_Polyhedron, ppl::Generator_System>::set_input_private(const Parma_Polyhedra_Library::C_Polyhedron& iv_input){
    *m_input_private = iv_input;
}

template <>
void Door<ppl::C_Polyhedron, ppl::Generator_System>::set_output_private(const Parma_Polyhedra_Library::C_Polyhedron& iv_output){
    *m_output_private = iv_output;
}

template <>
void Door<ppl::C_Polyhedron, ppl::Generator_System>::synchronize(){
    omp_set_lock(&m_lock_read);
    m_input_private->minimized_constraints();
    m_output_private->minimized_constraints();
    //    if(!m_input_private->contains(m_input_public) || !m_input_public.contains(*m_input_private)){
    //        m_update_input++;
    m_input_public = *m_input_private;
    //    }
    //    if(!m_output_private->contains(m_output_public) || !m_output_public.contains(*m_output_private)){
    //        m_update_output++;
    m_output_public = *m_output_private;
    //    }

    omp_unset_lock(&m_lock_read);
}

/// ******************  Other functions ****************** ///

ppl::C_Polyhedron iv_2_polyhedron(const ibex::IntervalVector& iv){
    Rational_Box box(iv.size());
    for(size_t i=0; i<box.space_dimension(); i++){
        ppl::Variable x(i);
        if(!iv[i].is_empty()){
            box.add_constraint(x >= floor(iv[i].lb()*IBEX_PPL_PRECISION));
            box.add_constraint(x <= ceil(iv[i].ub()*IBEX_PPL_PRECISION));
        }
    }
    return ppl::C_Polyhedron(box);
}

ibex::IntervalVector polyhedron_2_iv(const ppl::C_Polyhedron& p){
    ppl::Rational_Box box(p);
    ibex::IntervalVector result(p.space_dimension(), ibex::Interval::EMPTY_SET);

    for(size_t i=0; i<box.space_dimension(); i++){
        ppl::Variable x(i);
        result[i] = ibex::Interval(box.get_interval(x).lower().get_d()/IBEX_PPL_PRECISION, box.get_interval(x).upper().get_d()/IBEX_PPL_PRECISION);
    }
    return result;
}

}

