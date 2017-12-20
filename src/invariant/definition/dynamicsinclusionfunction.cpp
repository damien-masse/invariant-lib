#include "dynamicsinclusionfunction.h"

using namespace std;
using namespace ibex;

namespace invariant {
DynamicsInclusionFunction::DynamicsInclusionFunction(const std::vector<ibex::Function *> functions, ibex::IntervalVector inclusion_parameter, const DYNAMICS_SENS sens, bool taylor):
Dynamics_Function(functions, sens, taylor)
{
    m_inclusion_parameter = inclusion_parameter;
}

DynamicsInclusionFunction::DynamicsInclusionFunction(ibex::Function *f,  ibex::IntervalVector inclusion_parameter, const DYNAMICS_SENS sens, bool taylor):
Dynamics_Function(functions, sens, taylor)
{
    m_inclusion_parameter = inclusion_parameter;
}

const std::vector<ibex::IntervalVector> DynamicsInclusionFunction::eval(const ibex::IntervalVector &position){
    vector<IntervalVector> vector_field;
    IntervalVector val(position.size()+m_inclusion_parameter.size());
    for(size_t i=0; i<position.size(); i++)
        val[i] = position[i];
    for(size_t i=0; i<m_inclusion_parameter.size(); i++)
        val[i+position.size()] = m_inclusion_parameter[i];

    for(Function*f:m_functions){
        IntervalVector result = f->eval_vector(val);
        vector_field.push_back(result);
    }
    return vector_field;
}

const std::vector<ibex::IntervalMatrix> DynamicsInclusionFunction::eval_d1(const ibex::IntervalVector &position){

}

void DynamicsInclusionFunction::set_inclusion_parameter(ibex::IntervalVector parameter){
    m_inclusion_parameter = parameter;
}

ibex::IntervalVector DynamicsInclusionFunction::get_inclusion_parameter(){
    return m_inclusion_parameter;
}
}
