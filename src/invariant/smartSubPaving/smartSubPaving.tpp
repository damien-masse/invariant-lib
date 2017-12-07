#include "smartSubPaving.h"

namespace invariant {
template<typename _Tp, typename _V>
SmartSubPaving<_Tp, _V>::SmartSubPaving(const ibex::IntervalVector &space):
    m_position(space)
{
    m_dim = (unsigned char) space.size();

    // Create search space Pave
    invariant::Pave<_Tp, _V>* p = new invariant::Pave<_Tp, _V>(space, this);
    m_paves.push_back(p);

    // Create infinity Paves around search space
//    IntervalVector* result;
//    int n=space.complementary(result);

//    for (int i=0; i<n; i++) {
//        Pave* p_infinity = new Pave(result[i], this);
//        std::cout << result[i] << std::endl;
//        m_paves_not_bisectable.push_back(p_infinity);
//    }

    // Analyze faces (border)
    for(Pave<_Tp, _V> *p:m_paves)
        p->analyze_border();
    for(Pave<_Tp, _V> *p:m_paves_not_bisectable)
        p->analyze_border();

    // Root of the pave node tree
    m_tree = new Pave_node<_Tp, _V>(p);
    p->set_pave_node(m_tree);

    // Compute ratio dimension
    for(int dim=0; dim<m_dim; dim++){
        double diam = space[dim].diam();
        m_ratio_dimension.push_back(1.0/diam);
        m_limit_bisection.push_back(0.0);
    }
}

template<typename _Tp, typename _V>
SmartSubPaving<_Tp, _V>::~SmartSubPaving(){
    for(Pave<_Tp, _V> *p:m_paves){
        if(p!=NULL)
            delete(p);
    }
    delete(m_tree);
}

template<typename _Tp, typename _V>
void SmartSubPaving<_Tp, _V>::delete_pave(int id){
    delete(m_paves[id]);
    m_paves[id] = NULL;
}

template<typename _Tp, typename _V>
void SmartSubPaving<_Tp, _V>::serialize(std::ofstream& binFile) const{
    // unsigned char    dimension
    // size_t           number of paves
    // IntervalVector   position
    // [...] Paves of the graph

    binFile.write((const char*)&m_dim, sizeof(unsigned char)); // dimension
    size_t size = m_paves.size();
    binFile.write((const char*)&size, sizeof(size_t)); // Number of paves
    serializeIntervalVector(binFile, m_position);

    size_t cpt = 0;
    for(Pave<_Tp, _V> *p:m_paves){
        p->set_serialization_id(cpt); cpt++;
        p->serialize(binFile);
    }
}

template<typename _Tp, typename _V>
void SmartSubPaving<_Tp, _V>::deserialize(std::ifstream& binFile){
    if(m_paves.size()!=0){
        throw std::runtime_error("in [graph.cpp/deserialize] SmartSubPaving is not empty");
        return;
    }

    binFile.read((char*)&m_dim, sizeof(unsigned char));
    size_t number_pave;
    binFile.read((char*)&number_pave, sizeof(size_t));
    m_position = deserializeIntervalVector(binFile);

    const size_t number_pave_const = number_pave;
    for(size_t i=0; i<number_pave_const; i++){
        Pave<_Tp, _V> *p = new Pave<_Tp, _V>(this);
        m_paves.push_back(p);
    }
    for(size_t i=0; i<number_pave_const; i++){
        Pave<_Tp, _V> *p = m_paves[i];
        p->deserialize(binFile);
    }
}

template<typename _Tp, typename _V>
const bool SmartSubPaving<_Tp, _V>::is_equal(const SmartSubPaving<_Tp, _V>& g) const{
    if(m_position != g.get_position())
        return false;
    if(m_dim != g.dim())
        return false;
    const size_t nb_pave = m_paves.size();
    for(size_t i=0; i<nb_pave; i++){
        if(!(m_paves[i]->is_equal(*(g[i]))))
            return false;
    }
    return true;
}

template<typename _Tp, typename _V>
void SmartSubPaving<_Tp, _V>::bisect(){
    std::vector<Pave<_Tp, _V>*> m_bisectable_paves = m_paves;
    std::vector<Pave<_Tp, _V>*> m_bisected_paves;
    m_paves.clear();

    /// Bisect the graph ///
    while(m_bisectable_paves.size()>0){
        Pave<_Tp, _V> *p = m_bisectable_paves.back();
        m_bisectable_paves.pop_back();

        if(p->request_bisection()){
            p->bisect(); // bisected added to m_paves & update mazes
            delete(p);
        }
        else{
            // I
//            for(Maze *maze:m_mazes){
//                if(maze->get_type() == MAZE_WALL){
//                    Room *r = p->get_rooms()[maze];
//                    r->set_empty_private();
//                    r->synchronize();
//                }
//            }
            // Store not bisectable paves
            p->set_removed_rooms();
            m_paves_not_bisectable.push_back(p);
        }
    }

    /// Delete parent of bisected paves ///
    for(Pave<_Tp, _V>* p:m_bisected_paves)
        delete(p);
}

template<typename _Tp, typename _V>
void SmartSubPaving<_Tp, _V>::get_room_info(Maze<_Tp, _V> *maze, const ibex::IntervalVector& position, std::vector<Pave<_Tp, _V>*> &pave_list) const{
    m_tree->get_intersection_pave_outer(pave_list,position);
    for(Pave<_Tp, _V> *p:pave_list){
        Room<_Tp, _V> *r = p->get_rooms()[maze];
        std::cout << *r << std::endl;
    }
}


template<typename _Tp, typename _V>
std::pair<ibex::IntervalVector, ibex::IntervalVector> SmartSubPaving<_Tp, _V>::bisect_largest_first(const ibex::IntervalVector &position){
    // Select dimensions to bisect
    bool one_possible = false;
    std::vector<bool> possible_dim;
    for(int dim = 0; dim<m_dim; dim++){
        if(position[dim].diam() > m_limit_bisection[dim]){
            possible_dim.push_back(true);
            one_possible = true;
        }
        else{
            possible_dim.push_back(false);
        }
    }
    if(!one_possible){ // If no-one possible make all possible
        for(int dim=0; dim<m_dim; dim++)
            possible_dim[dim] = true;
    }

    // Find largest dimension
    ibex::Vector diam = position.diam();
    int dim_max = 0;
    double max = 0;
    for(int i=0; i<m_dim; i++){
        double test = diam[i]*m_ratio_dimension[i];
        if((max<test) & (possible_dim[i])){
            max = test;
            dim_max = i;
        }
    }
    ibex::IntervalVector p1(position);
    ibex::IntervalVector p2(position);

    p1[dim_max] = ibex::Interval(position[dim_max].lb(), position[dim_max].mid());
    p2[dim_max] = ibex::Interval(position[dim_max].mid(), position[dim_max].ub());

    return std::make_pair(p1, p2);
}

}