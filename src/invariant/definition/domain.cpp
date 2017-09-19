#include "domain.h"
#include "door.h"

using namespace ibex;
using namespace std;
namespace invariant {

Domain::Domain(Graph *graph, DOMAIN_PROPAGATION_START link){
    m_graph = graph;
    m_link_start = link;
}

void Domain::contract_domain(Maze *maze, std::vector<Room*> &list_room_deque){
    // ********** Separator contraction ********** //
    if(m_sep_output != NULL)
        contract_separator(maze, m_graph->get_tree(), list_room_deque, true, SEP_UNKNOWN); // Output
    if(m_sep_input != NULL)
        contract_separator(maze, m_graph->get_tree(), list_room_deque, false, SEP_UNKNOWN); // Input

    // ********** Border contraction ********** //
    contract_border(maze, list_room_deque);

    // ********** Add additional rooms to deque ********** //
    if(maze->get_type() == MAZE_DOOR){
        m_graph->get_tree()->get_all_child_rooms_not_empty(list_room_deque, maze);
    }
    if(maze->get_type() == MAZE_WALL && m_graph->get_paves().size()>1 && m_link_start==NOT_LINK_TO_INITIAL_CONDITION){ // When initial condition is not link with active paves
        //        m_graph->get_tree()->get_all_child_rooms_not_empty(list_room_deque, maze);
        m_graph->get_tree()->get_all_child_rooms_inside_outside(list_room_deque, maze);
        // (OK ?) Wrong function -> need to add neighbours of full paves instead of not_empty
    }
}

void Domain::contract_separator(Maze *maze, Pave_node *pave_node, std::vector<Room*> &list_room_deque, bool output, DOMAIN_SEP accelerator){
    MazeType type = maze->get_type();
    if(pave_node->get_removed()[maze])
        return;
    switch (accelerator) {
    case SEP_INSIDE:{
        if(!pave_node->get_fullness()[maze] || type==MAZE_WALL){
            if(pave_node->is_leaf()){
                Pave* p = pave_node->get_pave();
                Room *r = p->get_rooms()[maze];
                if(!r->is_removed()){
                    if(output)
                        r->set_full_private_output();
                    else
                        r->set_full_private_input();
                    if(type == MAZE_WALL)
                        p->get_neighbors_room(maze, list_room_deque);
                    r->synchronize();
                }
            }
            else{
                contract_separator(maze, pave_node->get_children().first, list_room_deque, output, SEP_INSIDE);
                contract_separator(maze, pave_node->get_children().second, list_room_deque, output, SEP_INSIDE);
            }
        }
    }
        break;
    case SEP_OUTSIDE:{
        if(!pave_node->get_emptyness()[maze] || type==MAZE_DOOR){
            if(pave_node->is_leaf()){
                Pave* p = pave_node->get_pave();
                Room *r = p->get_rooms()[maze];
                if(!r->is_removed()){
                    if(output)
                        r->set_empty_private_output();
                    else
                        r->set_empty_private_input();
                    r->synchronize();
                }
            }
            else{
                contract_separator(maze, pave_node->get_children().first, list_room_deque, output, SEP_OUTSIDE);
                contract_separator(maze, pave_node->get_children().second, list_room_deque, output, SEP_OUTSIDE);
            }
        }
    }
        break;
    case SEP_UNKNOWN:{
        IntervalVector x_in(pave_node->get_position());
        IntervalVector x_out(x_in);
        if(output)
            m_sep_output->separate(x_in, x_out);
        else
            m_sep_input->separate(x_in, x_out);

        if(pave_node->is_leaf()){
            Pave* p = pave_node->get_pave();
            Room *r = p->get_rooms()[maze];
            if(!r->is_removed()){
                if(x_in.is_empty()){ // Inside the constraint
                    if(output)
                        r->set_full_private_output();
                    else
                        r->set_full_private_input();
                    if(type == MAZE_WALL){
                        p->get_neighbors_room(maze, list_room_deque);
                        list_room_deque.push_back(r);
                    }
                }
                else if(x_out.is_empty()){  // Outside the constraint
                    if(output)
                        r->set_empty_private_output();
                    else
                        r->set_empty_private_input();
                }
                else{ // Inside & Outside the constraint => all full (over approximation)
                    if(output)
                        r->set_full_private_output();
                    else
                        r->set_full_private_input();
                    if(type == MAZE_WALL){
                        p->get_neighbors_room(maze, list_room_deque);
                        list_room_deque.push_back(r);
                    }
                }
                r->synchronize();
            }
        }
        else{
            // Determine the accelerator
            if(x_in.is_empty()){
                // Completly inside the constraint
                contract_separator(maze, pave_node->get_children().first, list_room_deque, output, SEP_INSIDE);
                contract_separator(maze, pave_node->get_children().second, list_room_deque, output, SEP_INSIDE);
            }
            else if(x_out.is_empty()){
                // Completly outside the constraint
                contract_separator(maze, pave_node->get_children().first, list_room_deque, output, SEP_OUTSIDE);
                contract_separator(maze, pave_node->get_children().second, list_room_deque, output, SEP_OUTSIDE);
            }
            else{
                // Mix area (outside & inside)
                contract_separator(maze,pave_node->get_children().first, list_room_deque, output, SEP_UNKNOWN);
                contract_separator(maze,pave_node->get_children().second, list_room_deque, output, SEP_UNKNOWN);
            }
        }
    }
        break;
    default:
        break;
    }
}

void Domain::contract_border(Maze *maze, std::vector<Room*> &list_room_deque){
    MazeType type = maze->get_type();

    if(m_graph->size()==1 && type == MAZE_DOOR)
        return;

    vector<Pave*> pave_border_list;
    m_graph->get_tree()->get_border_paves(pave_border_list);

    for(Pave *p:pave_border_list){
        Room *r = p->get_rooms()[maze];
        if(!r->is_removed()){
            for(Face *f:p->get_faces_vector()){
                if(f->is_border()){
                    Door *d = f->get_doors()[maze];
                    if(m_border_path_in)
                        d->set_full_private_input();
                    else{
                        if(type != MAZE_WALL)
                            d->set_empty_private_input();
                    }

                    if(m_border_path_out)
                        d->set_full_private_output();
                    else{
                        if(type != MAZE_WALL)
                            d->set_empty_private_output();
                    }
                    d->synchronize();
                }
            }

            if(type == MAZE_WALL && (m_border_path_in || m_border_path_out)){
                if(!p->get_rooms()[maze]->is_full())
                    list_room_deque.push_back(p->get_rooms()[maze]);
            }
            if(type == MAZE_DOOR && (!m_border_path_in || !m_border_path_out)){
                if(!p->get_rooms()[maze]->is_empty())
                    list_room_deque.push_back(p->get_rooms()[maze]);
            }
        }
    }
}

void Domain::inter_maze(Maze *maze){
    for(Maze *maze_inter : m_maze_list){
        if(maze != maze_inter){

            std::vector<Room *> room_list;
            m_graph->get_tree()->get_all_child_rooms_not_empty(room_list, maze);

            for(Room *r:room_list){
                Pave *p = r->get_pave();
                Room *r_inter = p->get_rooms()[maze_inter];
                if(r_inter->is_empty()){
                    r->set_empty_private();
                    r->synchronize();
                }
            }
        }
    }
}

}
