#include "nodecurrent.h"

using namespace ibex;
using namespace std;

NodeCurrent::NodeCurrent(const ibex::IntervalVector &position, double epsilon_bisection):
    m_position(position), m_vector_field(position.size(), Interval::EMPTY_SET)
{
    if(m_position.max_diam()<epsilon_bisection){
        m_leaf = true;
        m_leaf_list.push_back(this);
    }
    else{
        m_leaf = false;
        ibex::LargestFirst bisector(0, 0.5);
        std::pair<IntervalVector, IntervalVector> result_boxes = bisector.bisect(position);
        NodeCurrent *nc1 = new NodeCurrent(result_boxes.first, epsilon_bisection);
        NodeCurrent *nc2 = new NodeCurrent(result_boxes.second, epsilon_bisection);
        m_children.first = nc1;
        m_children.second = nc2;
        m_leaf_list.insert(m_leaf_list.end(), nc1->get_leaf_list().begin(), nc1->get_leaf_list().end());
        m_leaf_list.insert(m_leaf_list.end(), nc2->get_leaf_list().begin(), nc2->get_leaf_list().end());
    }
}

const ibex::IntervalVector& NodeCurrent::compute_vector_field_tree(){
    if(m_leaf)
        return m_vector_field;
    else{
        m_vector_field = m_children.first->compute_vector_field_tree();
        m_vector_field |= m_children.second->compute_vector_field_tree();
        return m_vector_field;
    }
}

const ibex::IntervalVector NodeCurrent::eval(const IntervalVector& position){
    if(m_leaf){
        return m_vector_field;
    }
    else{
        IntervalVector inter = position & m_position;
        if(inter.is_empty()){
            IntervalVector empty(position.size(), Interval::EMPTY_SET);
            return empty;
        }
        else if(position.is_subset(m_position)){
            return m_vector_field;
        }
        else{
            IntervalVector result(position.size(), Interval::EMPTY_SET);
            result |= m_children.first->eval(inter);
            result |= m_children.second->eval(inter);
            return result;
        }
    }
}

void NodeCurrent::fill_leafs(short *raw_u, short *raw_v, const size_t& i_max, const float& scale_factor, const short& fill_value){

    int nb_node=m_leaf_list.size();
    size_t dim = m_position.size();

//    #pragma omp parallel for
    for(int id=0; id<nb_node; id++){
        NodeCurrent *nc = m_leaf_list[id];

        vector<vector<int>> tab_point;
        for(size_t d = 0; d<dim; d++){
            vector<int> pt;
            pt.push_back(ceil(nc->get_position()[d].mid()));
            pt.push_back(floor(nc->get_position()[d].mid()));
            tab_point.push_back(pt);
        }

        IntervalVector vector_field(dim, Interval::EMPTY_SET);
        bool no_value = false;
        // U
        for(size_t k=0; k<tab_point[0].size(); k++){
            for(size_t l=0; l<tab_point[0].size(); l++){
                size_t j_coord = tab_point[0][k];
                size_t i_coord = tab_point[1][l];
                short v = raw_u[i_max*j_coord+i_coord];
                if(v!=fill_value)
                    vector_field[0] |= Interval(v*scale_factor);
                else
                    no_value = true;
            }
        }

        // V
        for(size_t k=0; k<tab_point[0].size(); k++){
            for(size_t l=0; l<tab_point[0].size(); l++){
                int j_coord = tab_point[0][k];
                int i_coord = tab_point[1][l];
                short v = raw_v[i_max*j_coord+i_coord];
                if(v!=fill_value)
                    vector_field[1] |= Interval(v*scale_factor);
                else
                    no_value = true;
            }
        }
        if(no_value)
            vector_field = IntervalVector(2, Interval::ZERO);

        nc->set_vector_field(vector_field);
    }
}
