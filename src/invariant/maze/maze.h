#ifndef MAZE_H
#define MAZE_H

#include <ibex_IntervalVector.h>
#include <ppl.hh>

#include <deque>
#include <iostream>

#include <omp.h>

#include "dynamics.h"
#include "domain.h"
#include "room.h"
#include "smartSubPaving.h"
#include "pave.h"

namespace invariant {

template <typename _Tp> class Maze;
using MazePPL = Maze<Parma_Polyhedra_Library::C_Polyhedron>;
using MazeIBEX = Maze<ibex::IntervalVector>;

class Dynamics; // declared only for friendship
template <typename _Tp> class Room;
template <typename _Tp> class Domain;
template <typename _Tp> class SmartSubPaving;
template <typename _Tp> class Pave;
template <typename _Tp> class Face;
template <typename _Tp> class Door;

template <typename _Tp=ibex::IntervalVector>
class Maze
{
public:
    /**
     * @brief Constructor of a Maze
     * @param g
     * @param f_vect
     * @param type of operation (forward, backward or both)
     */
    Maze(invariant::Domain<_Tp> *domain, Dynamics *dynamics);

    /**
     * @brief Maze destructor
     */
    ~Maze();

    /**
     * @brief Getter to the Domain
     * @return
     */
    Domain<_Tp> * get_domain() const;

    /**
     * @brief Getter to the SmartSubPaving
     * @return
     */
    SmartSubPaving<_Tp> * get_subpaving() const;

    /**
     * @brief Getter to the dynamics
     * @return
     */
    Dynamics *get_dynamics() const;

    /**
     * @brief Contract the Maze
     * @return the number of contractions
     */
    int contract(size_t nb_operations=0);

    /**
     * @brief Contract the Maze by intersecting with other domain mazes
     */
//    void contract_inter(Maze *maze_n);

    /**
     * @brief Add a Room to the deque BUT DO NOT CHECK if already in
     * (to short locking the deque)
     * @param r
     */
    void add_to_deque(Room<_Tp> *r);

    /**
     * @brief Add a list of rooms to the deque (check if already in)
     * @param list_rooms
     */
    void add_rooms(const std::vector<Room<_Tp> *> &list_rooms);

    /**
     * @brief Return true if some trajectory can escape from the search space
     * @return
     */
    bool is_escape_trajectories();

    /**
     * @brief Return true if all rooms of the maze are empty
     * @return
     */
    bool is_empty();

    /**
     * @brief get the bounding box
     * @return
     */
    ibex::IntervalVector get_bounding_box();

    /**
     * @brief compute_vector_field
     */
    void compute_vector_field();

    /**
     * @brief get widening limit
     * @return
     */
    size_t get_widening_limit() const;

    /**
     * @brief get contraction limit
     * @return
     */
    size_t get_contraction_limit() const;

    /**
     * @brief get limit contraction door
     * @return
     */
    bool get_limit_contraction_door() const;

    /**
     * @brief set widening limit
     * @param limit
     */
    void set_widening_limit(size_t limit);

    /**
     * @brief set contraction limit
     * @param limit
     */
    void set_contraction_limit(size_t limit);

    /**
     * @brief set a limit to door contraction (equivalent widening)
     * @param val
     */
    void set_enable_contraction_limit(bool val);

    /**
     * @brief Set nb_operation to 0
     */
    void reset_nb_operations();

    /**
     * @brief Set enable contract domain variable
     * @param val
     */
    void set_enable_contract_domain(bool val);

    /**
     * @brief add_initial_room
     * @param room_pt
     */
    void add_initial_room(Room<_Tp>* room_pt);

    /**
     * @brief reset_initial_room_list
     */
    void reset_initial_room_list();

    /**
     * @brief get_initial_room_list
     * @return
     */
    std::vector<Room<_Tp>*> get_initial_room_list();

    /**
     * @brief get_contract_once
     * @return
     */
    bool get_contract_once() const;

private:
    invariant::Domain<_Tp> *    m_domain = nullptr;
    SmartSubPaving<_Tp>  *    m_subpaving = nullptr; // SmartSubPaving associated with this maze
    Dynamics *  m_dynamics = nullptr;

    std::deque<Room<_Tp> *> m_deque_rooms;
    omp_lock_t  m_deque_access;

    bool    m_espace_trajectories = true;
    bool    m_contract_once = false;
    bool m_empty = false;
    int m_contraction_step = 0;

    size_t m_nb_operations=0;
    size_t m_widening_limit=1000;
    size_t m_contraction_limit=1000;
    bool m_enable_contract_domain = true;
    bool m_limit_contraction_door = false;

    std::vector<Room<_Tp>*> m_initial_rooms_list;
};
}

namespace invariant{

template <typename _Tp>
inline void Maze<_Tp>::set_widening_limit(size_t limit){
    m_widening_limit = limit;
}

template <typename _Tp>
inline void Maze<_Tp>::set_contraction_limit(size_t limit){
    m_contraction_limit = limit;
}

template <typename _Tp>
inline void Maze<_Tp>::set_enable_contraction_limit(bool val){
    m_limit_contraction_door = val;
}

template <typename _Tp>
inline void Maze<_Tp>::set_enable_contract_domain(bool val){
    m_enable_contract_domain = val;
}

template <typename _Tp>
inline size_t Maze<_Tp>::get_widening_limit() const{
    return m_widening_limit;
}

template <typename _Tp>
inline size_t Maze<_Tp>::get_contraction_limit() const{
    return m_contraction_limit;
}

template <typename _Tp>
inline bool Maze<_Tp>::get_limit_contraction_door() const{
    return m_limit_contraction_door;
}

template <typename _Tp>
inline Domain<_Tp> * Maze<_Tp>::get_domain() const{
    return m_domain;
}

template <typename _Tp>
inline Dynamics * Maze<_Tp>::get_dynamics() const{
    return m_dynamics;
}

template <typename _Tp>
inline SmartSubPaving<_Tp> * Maze<_Tp>::get_subpaving() const{
    return m_subpaving;
}

template <typename _Tp>
inline void Maze<_Tp>::add_to_deque(Room<_Tp> *r){
    omp_set_lock(&m_deque_access);
    m_deque_rooms.push_back(r);
    omp_unset_lock(&m_deque_access);
}

template <typename _Tp>
inline void Maze<_Tp>::add_initial_room(Room<_Tp>* room_pt){
    m_initial_rooms_list.push_back(room_pt);
}

template <typename _Tp>
inline void Maze<_Tp>::reset_initial_room_list(){
    m_initial_rooms_list.clear();
}

template <typename _Tp>
inline std::vector<Room<_Tp>*> Maze<_Tp>::get_initial_room_list(){
    return m_initial_rooms_list;
}

template <typename _Tp>
inline bool Maze<_Tp>::get_contract_once() const{
    return m_contract_once;
}

template <typename _Tp>
Parma_Polyhedra_Library::Thread_Init* initialize_thread();

template <typename _Tp>
void delete_thread_init(Parma_Polyhedra_Library::Thread_Init* thread_init);

}

#include "maze.tpp"

#endif // MAZE_H
