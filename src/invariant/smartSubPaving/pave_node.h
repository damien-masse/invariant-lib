#ifndef PAVE_NODE_H
#define PAVE_NODE_H

#include <utility>

#include <ibex_IntervalVector.h>
#include <ibex_Ctc.h>

#include "pave.h"
#include "face.h"
#include "../maze/maze.h"
#include "../maze/room.h"

#include "pave_node.h"
#include "room.h"

namespace invariant{

template <typename _Tp, typename _V> class Pave_node;
using Pave_nodePPL = Pave_node<Parma_Polyhedra_Library::C_Polyhedron, Parma_Polyhedra_Library::Generator_System>;
using Pave_nodeIBEX = Pave_node<ibex::IntervalVector, ibex::IntervalVector>;

template <typename _Tp, typename _V> class Pave;
template <typename _Tp, typename _V> class Maze;
template <typename _Tp, typename _V> class Face;
template <typename _Tp, typename _V> class Room;

template <typename _Tp=ibex::IntervalVector, typename _V=ibex::IntervalVector>
class Pave_node
{
public:
    /**
     * @brief Construct a leaf node of the tree which leans to the subpaving
     * @param p
     */
    Pave_node(Pave<_Tp, _V> *p);

    /**
     * @brief Recursive destructor
     */
    ~Pave_node();

    /**
     * @brief Add childs to the binary tree (this node is set to not be a leaf)
     * @param Pave1
     * @param Pave2
     */
    void add_child(Pave<_Tp, _V> *p1, Pave<_Tp, _V> *p2);

    /**
     * @brief Get all Paves that intersect a box (outer condition)
     * @param vector of Pave*
     * @param box
     */
    void get_intersection_pave_outer(std::vector<Pave<_Tp, _V>*> &l, const ibex::IntervalVector &box);

    /**
     * @brief Get all Paves that are not contracted to emptyness by a given contractor
     * @param vector of Pave*
     * @param NumConstraint to fulfill (>0)
     */
    void get_intersection_pave_outer(std::vector<Pave<_Tp, _V>*> &l, ibex::Ctc &nc);

    /**
     * @brief Get all Paves that are inside a box (inner condition)
     * @param vector of Pave*
     * @param box
     */
    void get_intersection_pave_inner(std::vector<Pave<_Tp, _V>*> &l, const ibex::IntervalVector &box);

    /**
     * @brief Get all Paves that are not contracted by a given contractor
     * @param l
     * @param nc
     */
    void get_intersection_pave_inner(std::vector<Pave<_Tp, _V>*> &l, ibex::Ctc &nc);

    /**
     * @brief Get all Faces that intersect a box (outer condition)
     * @param vector of Pave*
     * @param box
     */
    void get_intersection_face_outer(std::vector<Face<_Tp, _V>*> &l, const ibex::IntervalVector &box);

    /**
     * @brief Get all Faces that are not contracted to emptyness by a given contractor
     * @param vector of Pave*
     * @param NumConstraint to fulfill (>0)
     */
    void get_intersection_face_outer(std::vector<Face<_Tp, _V>*> &l, ibex::Ctc &nc);

    /**
     * @brief Get all Faces that are inside a box (inner condition)
     * @param vector of Pave*
     * @param box
     */
    void get_intersection_face_inner(std::vector<Face<_Tp, _V>*> &l, const ibex::IntervalVector &box);

    /**
     * @brief Get all Faces that are not contracted by a given contractor
     * @param l
     * @param nc
     */
    void get_intersection_face_inner(std::vector<Face<_Tp, _V>*> &l, ibex::Ctc &nc);

    /**
     * @brief Get the pave associated to this pave_node
     * @return
     */
    Pave<_Tp, _V>* get_pave() const;

    /**
     * @brief Return true is this Pave_node is a leaf
     * @return
     */
    bool is_leaf() const;

    /**
     * @brief Getter to the position of this pave_node
     * @return
     */
    const ibex::IntervalVector get_position() const;

    /**
     * @brief Get all the room of the children of this pave_node
     * @param list_room
     * @param maze
     */
    void get_all_child_rooms(std::vector<Room<_Tp, _V> *> &list_room, Maze<_Tp, _V> *maze) const;

    /**
     * @brief Get all non empty room of the children of this pave_node
     * @param list_room
     * @param maze
     */
    void get_all_child_rooms_not_empty(std::vector<Room<_Tp, _V> *> &list_room, Maze<_Tp, _V> *maze) const;

    /**
     * @brief Get all non full room of the children of this pave_node
     * @param list_room
     * @param maze
     */
    void get_all_child_rooms_not_full(std::vector<Room<_Tp, _V> *> &list_room, Maze<_Tp, _V> *maze) const;

    /**
     * @brief Get all outside rooms that are on the border of the set
     * @param list_room
     * @param maze
     */
    void get_all_child_rooms_inside_outside(std::vector<Room<_Tp, _V> *> &list_room, Maze<_Tp, _V> *maze) const;

    /**
     * @brief Getter to the children of this pave_node
     * @return
     */
    const std::pair<Pave_node<_Tp, _V> *, Pave_node<_Tp, _V> *> get_children() const;

    /**
     * @brief get bounding fullness
     * @return
     */
    void get_bounding_fullness(Maze<_Tp, _V> *maze, ibex::IntervalVector &result);

    /**
     * @brief Add a flag to the emptyness map to tell if a room was empty at that level of the tree
     * @param maze
     * @param empty
     */
    void add_emptyness(Maze<_Tp, _V> *maze, bool empty);

    /**
     * @brief Add a flag to the fullness map to tell if a room was full at that level of the tree
     * @param maze
     * @param empty
     */
    void add_fullness(Maze<_Tp, _V> *maze, bool full);

    /**
     * @brief Add a flag to the removed map to tell if a room was removed at that level of the tree
     * @param maze
     * @param removed
     */
    inline void add_removed(Maze<_Tp, _V> *maze, bool removed);

    /**
     * @brief Return the map of emptyness
     * @return
     */
    std::map<Maze<_Tp, _V>*, bool> get_emptyness() const;

    /**
     * @brief Return the map of fullness
     * @return
     */
    std::map<Maze<_Tp, _V>*, bool> get_fullness() const;

    /**
     * @brief Return the map of removed
     * @return
     */
    std::map<Maze<_Tp, _V> *, bool> get_removed() const;

    /**
     * @brief Set removed pave_node
     * @param Maze
     */
    void set_removed(Maze<_Tp, _V> *maze);


    /**
     * @brief Return true if this pave_node is a border pave
     * @return
     */
    bool is_border_pave() const;

    /**
     * @brief Get the list of border paves
     * @param pave_list
     */
    void get_border_paves(std::vector<Pave<_Tp, _V>*> &pave_list) const;

    /**
     * @brief Get not empty rooms inside the box that intersect the polygon
     * @param l
     * @param box
     * @param maze
     */
    void get_intersection_polygon_not_empty(std::vector<Room<_Tp, _V>*> &l, const ibex::IntervalVector &box, Maze<_Tp, _V> *maze) const;

    /**
     * @brief Get empty rooms inside the box that intersect the polygon
     * @param l
     * @param box
     * @param maze
     */
    void get_intersection_polygon_empty(std::vector<Room<_Tp, _V>*> &l, const ibex::IntervalVector &box, Maze<_Tp, _V> *maze) const;

private:
    bool                                m_leaf = true;
    mutable Pave<_Tp, _V>*                       m_pave = NULL;
    std::pair<Pave_node<_Tp, _V> *, Pave_node<_Tp, _V> *> m_children;
    const ibex::IntervalVector          m_position;

    std::map<Maze<_Tp, _V>*, bool>               m_empty_rooms;
    std::map<Maze<_Tp, _V>*, bool>               m_full_rooms;
    std::map<Maze<_Tp, _V>*, bool>               m_removed_rooms;
    bool                                m_border_pave;
};

}

namespace invariant {
template<typename _Tp, typename _V>
inline bool Pave_node<_Tp, _V>::is_leaf() const{
    return m_leaf;
}

template<typename _Tp, typename _V>
inline Pave<_Tp, _V>* Pave_node<_Tp, _V>::get_pave() const{
    if(m_leaf)
        return m_pave;
    else{
        throw std::runtime_error("in [pave_node.cpp/get_pave] this Pave_node is not a leaf");
        return NULL;
    }
}

template<typename _Tp, typename _V>
inline const ibex::IntervalVector Pave_node<_Tp, _V>::get_position() const{
    return m_position;
}

template<typename _Tp, typename _V>
inline const std::pair<Pave_node<_Tp, _V> *, Pave_node<_Tp, _V> *> Pave_node<_Tp, _V>::get_children() const{
    return m_children;
}

template<typename _Tp, typename _V>
inline void Pave_node<_Tp, _V>::add_emptyness(Maze<_Tp, _V> *maze, bool empty){
    m_empty_rooms.insert(std::make_pair(maze, empty));
}

template<typename _Tp, typename _V>
inline void Pave_node<_Tp, _V>::add_removed(Maze<_Tp, _V> *maze, bool removed){
    m_removed_rooms.insert(std::make_pair(maze, removed));
}

template<typename _Tp, typename _V>
inline void Pave_node<_Tp, _V>::add_fullness(Maze<_Tp, _V> *maze, bool empty){
    m_full_rooms.insert(std::make_pair(maze, empty));
}

template<typename _Tp, typename _V>
inline std::map<Maze<_Tp, _V>*, bool> Pave_node<_Tp, _V>::get_emptyness() const{
    return m_empty_rooms;
}

template<typename _Tp, typename _V>
inline std::map<Maze<_Tp, _V>*, bool> Pave_node<_Tp, _V>::get_fullness() const{
    return m_full_rooms;
}

template<typename _Tp, typename _V>
inline std::map<Maze<_Tp, _V> *, bool> Pave_node<_Tp, _V>::get_removed() const{
    return m_removed_rooms;
}

template<typename _Tp, typename _V>
inline void Pave_node<_Tp, _V>::set_removed(Maze<_Tp, _V> *maze){
    m_removed_rooms[maze] = true;
}

template<typename _Tp, typename _V>
inline bool Pave_node<_Tp, _V>::is_border_pave() const{
    return m_border_pave;
}

/// ******************  Sepcialized ****************** ///

}

#include "pave_node.tpp"

#endif // PAVE_NODE_H
