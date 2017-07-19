#include "previmer3d.h"
#include "nodecurrent3d.h"
#include <netcdf>
#include <omp.h>

#include <sys/types.h>
#include <dirent.h>

#include <utility>

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;
using namespace ibex;

namespace invariant {

int parseLine(char* line){
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

int getValue(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmRSS:", 6) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}

PreviMer3D::PreviMer3D(const std::string& file_directory, const IntervalVector &search_space, std::vector<double> grid_size, const std::vector<double> &limit_bisection, int stop_level){
    int ram_init = getValue()/1000;
    cout << "Mem 0 = " << ram_init << " Mo" << endl;

    if(search_space.size()!=3){
        throw std::runtime_error("in [previmer3d.cpp/PreviMer3D()] dim of search_space is not equal to 3");
    }
    m_dim = 3;
    m_grid_size = grid_size;
    m_limit_bisection = limit_bisection;

    // Compute ratio dimension
    for(int dim=0; dim<m_dim; dim++){
        double diam = search_space[dim].diam();
        m_ratio_dimension.push_back(1.0/diam);
    }

    vector<string> file_list;
    get_file_list(file_directory, file_list);
    int nb_files = file_list.size();
    int t_max = min(nb_files, (int)(search_space[0].ub()/grid_size[0]));
    int t_min = max(0, (int)(search_space[0].lb()/grid_size[0]));
    bool first_read = true;
    int cpt = 0;

    std::vector<std::vector<std::vector<short>>> *raw_u_t = new std::vector<std::vector<std::vector<short>>>;
    std::vector<std::vector<std::vector<short>>> *raw_v_t = new std::vector<std::vector<std::vector<short>>>;

    for(int file_id = t_min; file_id<t_max; file_id++){
        const string file_name = file_list[file_id];

        cout << "load file : " << ++cpt << "/" << t_max << " T+" << floor(cpt*0.25) << "h" << (cpt*15)%60 << "min" << endl;
        NcFile dataFile(file_directory+file_name, NcFile::read);

        NcVar u_var=dataFile.getVar("U");
        NcVar v_var=dataFile.getVar("V");

        // ******* Size U,V ******
        size_t j_max = dataFile.getDim("nj_u").getSize();
        size_t i_max = dataFile.getDim("ni_u").getSize();

        // ******* DATA U,V ******
        short *raw_u = new short[j_max*i_max];
        short *raw_v = new short[j_max*i_max];
        u_var.getVar(raw_u);
        v_var.getVar(raw_v);

        vector<vector<short>> tab_u, tab_v;

        for(size_t i=0; i<i_max; i++){
            vector<short> line_u, line_v;
            for(size_t j=0; j<j_max; j++){
                line_u.push_back(raw_u[i_max*j+i]);
                line_v.push_back(raw_v[i_max*j+i]);
            }
            tab_u.push_back(line_u);
            tab_v.push_back(line_v);
        }

        delete[] raw_u;
        delete[] raw_v;

        raw_u_t->push_back(tab_u);
        raw_v_t->push_back(tab_v);

        if(first_read){
            u_var.getAtt("scale_factor").getValues(&m_scale_factor);
            u_var.getAtt("_FillValue").getValues(&m_fill_value);
            first_read = false;
            m_size.push_back(nb_files);
            m_size.push_back(i_max);
            m_size.push_back(j_max);
        }
    }
    cout << "Data properties" << endl;
    cout << " Scale factor = " << m_scale_factor << endl;
    cout << " Fill Value = " << m_fill_value << endl;

//    // ******* Node Current ******
    IntervalVector position(search_space);
    if(search_space.is_unbounded()){
        for(int i=0; i<3; i++)
            position[i] = Interval(0, m_size[i]*m_grid_size[i]);
        cout << "unbound search space set to : " << position << endl;
    }

    double time_start_init = omp_get_wtime();
    cout << "Mem 1 = " << getValue()/1000 - ram_init << " Mo" << endl;  // 29 Mo

    cout << "TIME build tree = ";
    m_node_current = new NodeCurrent3D(position, limit_bisection, this, m_leaf_list);
    cout << omp_get_wtime() - time_start_init << endl;

    time_start_init = omp_get_wtime();
    cout << "Nb leafs = " << m_leaf_list.size() << endl;

    cout << "Mem 2 = " << getValue()/1000 - ram_init << " Mo" << endl;  // 1887 Mo (2^22)

    cout << "TIME compute leafs = ";
    this->fill_leafs(*raw_u_t, *raw_v_t);
    cout << omp_get_wtime() - time_start_init << endl;

    cout << "Mem 3 = " << getValue()/1000 - ram_init<< " Mo" << endl;

    delete(raw_u_t);
    delete(raw_v_t);

    time_start_init = omp_get_wtime();
    m_node_current->compute_vector_field_tree();
    cout << "TIME compute tree = " << omp_get_wtime() - time_start_init << endl;

    cout << "Mem 4 = " << getValue()/1000 - ram_init << " Mo" << endl;
}

PreviMer3D::~PreviMer3D(){
    delete(m_node_current);
}

const vector<ibex::IntervalVector> PreviMer3D::eval(const ibex::IntervalVector& position){
    vector<ibex::IntervalVector> vector_fields;
    vector_fields.push_back(m_node_current->eval(position));
    return vector_fields;
}

const ibex::IntervalVector& PreviMer3D::get_search_space(){
    return m_node_current->get_position();
}

int PreviMer3D::get_file_list(string dir, vector<string> &files){
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        cout << "Error(" << errno << ") opening " << dir << endl;
        return errno;
    }

    while ((dirp = readdir(dp)) != NULL) {
        string file_name = string(dirp->d_name);
        if(!(file_name == "." || file_name == ".."))
            files.push_back(file_name);
    }
    closedir(dp);
    return 0;
}

std::pair<IntervalVector, IntervalVector> PreviMer3D::bisect_largest_first(const IntervalVector &position){
    // Select dimensions to bisect
    bool one_possible = false;
    vector<bool> possible_dim;
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
    Vector diam = position.diam();
    int dim_max = 0;
    double max = 0;
    for(int i=0; i<m_dim; i++){
        double test = diam[i]*m_ratio_dimension[i];
        if((max<test) & (possible_dim[i])){
            max = test;
            dim_max = i;
        }
    }
    IntervalVector p1(position);
    IntervalVector p2(position);

    p1[dim_max] = Interval(position[dim_max].lb(), position[dim_max].mid());
    p2[dim_max] = Interval(position[dim_max].mid(), position[dim_max].ub());

    return std::make_pair(p1, p2);
}

void PreviMer3D::fill_leafs(const std::vector<std::vector<std::vector<short> > > &raw_u_t, const std::vector<std::vector<std::vector<short> > > &raw_v_t){

    int nb_node=m_leaf_list.size();
    int d_max[3] = {(int)(raw_u_t.size())-1, (int)(raw_u_t[0].size())-1, (int)(raw_u_t[0][0].size())-1};

#pragma omp parallel for
    for(int id=0; id<nb_node; id++){
        NodeCurrent3D *nc = m_leaf_list[id];

        /// **** Interpolation of vector field ****
        vector<vector<int>> tab_point_u, tab_point_v;

        for(int dim = 0; dim<3; dim++){
            vector<int> pt_u, pt_v;
            Interval center_u = nc->get_position()[dim].mid() / m_grid_size[dim]; // Back to the grid coord
            Interval center_v;
            if(dim > 0)
                center_v = (nc->get_position()[dim].mid()+0.5*m_grid_size[dim]) / m_grid_size[dim]; // Back to the grid coord
            else
                center_v = nc->get_position()[dim].mid() / m_grid_size[dim]; // Back to the grid coord

            // Cross pattern
            for(int i=std::max(0, std::min((int)floor(center_u.lb()), d_max[dim]));
                i<std::max(0, std::min((int)ceil(center_u.ub()), d_max[dim]));
                i++){
                pt_u.push_back(i);
            }
            for(int i=std::max(0, std::min((int)floor(center_v.lb()), d_max[dim]));
                i<std::max(0, std::min((int)ceil(center_v.ub()), d_max[dim]));
                i++){
                pt_v.push_back(i);
            }
            tab_point_u.push_back(pt_u);
            tab_point_v.push_back(pt_v);
        }

        IntervalVector vector_field(3, Interval::EMPTY_SET); // 3 Dimensions (dt, U, V)
        // T
//        vector_field[0] = Interval(0.9, 1.1);
        vector_field[0] = Interval(10.0, 10.1);
        bool no_value = false;
        // U
        for(size_t t_id=0; t_id<tab_point_u[0].size(); t_id++){
            for(size_t u_id=0; u_id<tab_point_u[0].size(); u_id++){
                for(size_t v_id=0; v_id<tab_point_u[0].size(); v_id++){
                    size_t t_coord = tab_point_u[0][t_id];
                    size_t i_coord = tab_point_u[1][u_id];
                    size_t j_coord = tab_point_u[2][v_id];

                    short vec_u = raw_u_t[t_coord][i_coord][j_coord];
                    if(vec_u!=m_fill_value){
                        vector_field[1] |= Interval(vec_u*m_scale_factor);
                    }
                    else
                        no_value = true;
                }
            }
        }
        // V
        for(size_t t_id=0; t_id<tab_point_v[0].size(); t_id++){
            for(size_t u_id=0; u_id<tab_point_v[0].size(); u_id++){
                for(size_t v_id=0; v_id<tab_point_v[0].size(); v_id++){
                    size_t t_coord = tab_point_v[0][t_id];
                    size_t i_coord = tab_point_v[1][u_id];
                    size_t j_coord = tab_point_v[2][v_id];

                    short vec_v = raw_v_t[t_coord][i_coord][j_coord];
                    if(vec_v!=m_fill_value){
                        vector_field[2] |= Interval(vec_v*m_scale_factor);
                    }
                    else
                        no_value = true;
                }
            }
        }

        if(no_value)
            vector_field = IntervalVector(3, Interval::EMPTY_SET);

//        nc->set_vector_field(vector_field);
    }
}

}
