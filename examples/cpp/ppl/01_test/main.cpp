#include <iostream>
#include <errno.h>
#include <ppl.hh>
#include <ibex.h>
#include <iostream>
#include <omp.h>

#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkDelaunay3D.h>
#include <vtkUnstructuredGrid.h>
#include <vtkSmartPointer.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkCellArray.h>
#include <vtkDataSetSurfaceFilter.h>

#include "resultstorage.h"

using namespace std;
namespace PPL = Parma_Polyhedra_Library;
using namespace Parma_Polyhedra_Library::IO_Operators;
using namespace ibex;

#define IBEX_PPL_PRECISION 1e16

PPL::Rational_Box iv_2_box(const ibex::IntervalVector &iv){
    Rational_Box box(iv.size());
    for(int i=0; i<(int)box.space_dimension(); i++){
        PPL::Variable x(i);

        if(!iv[i].is_empty()){
            box.add_constraint(x >= floor(iv[i].lb()*IBEX_PPL_PRECISION));
            box.add_constraint(x <= ceil(iv[i].ub()*IBEX_PPL_PRECISION));
        }
    }
    return box;
}

std::vector<PPL::Rational_Box> iv_2_box(const std::vector<ibex::IntervalVector> &list){
    std::vector<PPL::Rational_Box> list_RB;
    for(auto &iv:list){
        list_RB.push_back(iv_2_box(iv));
    }
    return list_RB;
}

void recursive_linear_expression_from_iv(const ibex::IntervalVector &theta,
                                         int dim,
                                         std::vector<Linear_Expression> &linear_expression_list,
                                         Linear_Expression &local_linear_expression){
    if(dim > 0){
        PPL::Variable x(dim-1);
        Linear_Expression l_b = local_linear_expression;
        Linear_Expression l_u = local_linear_expression;

        // ToDo: case theta[dim] -> lb=+oo | ub=-oo
        if(std::isinf(theta[dim-1].ub()))
            linear_expression_list.push_back(Linear_Expression(x));
        else
            l_u += x*ceil(theta[dim-1].ub()*IBEX_PPL_PRECISION);

        if(std::isinf(theta[dim-1].lb()))
            linear_expression_list.push_back(Linear_Expression(-x));
        else
            l_b += x*floor(theta[dim-1].lb()*IBEX_PPL_PRECISION);

        recursive_linear_expression_from_iv(theta, dim-1, linear_expression_list, l_u);
        recursive_linear_expression_from_iv(theta, dim-1, linear_expression_list, l_b);
    }
    else{
        linear_expression_list.push_back(local_linear_expression);
    }
}

PPL::C_Polyhedron add_ray(const ibex::IntervalVector &theta, const PPL::C_Polyhedron &p){
    Linear_Expression e = Linear_Expression(0);
    std::vector<Linear_Expression> linear_expression_list;
    PPL::C_Polyhedron p_copy(p);

    recursive_linear_expression_from_iv(theta, theta.size(), linear_expression_list,e);
    for(auto &l:linear_expression_list){
        p_copy.add_generator(ray(l));
    }
    return p_copy;
}

void write_VTK(const std::vector<Parma_Polyhedra_Library::C_Polyhedron> &ph_list, string filename){
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer< vtkPoints >::New();

    for(auto &ph:ph_list){
        for(auto &g:ph.generators()){
            if(g.is_point()){
                std::vector<double> coord;
                if(ph.space_dimension()<3){
                    for(int cpt = 0; cpt < 2; cpt++){
                        coord.clear();
                        for(int i=0; i<3; i++){
                            PPL::Variable x(i);
                            if(i<2){
                                cout << g.coefficient(x).get_d()/(g.divisor().get_d()*IBEX_PPL_PRECISION) << ",";
                                coord.push_back(g.coefficient(x).get_d()/(g.divisor().get_d()*IBEX_PPL_PRECISION));
                            }
                            else{
                                cout << cpt << endl;
                                coord.push_back(cpt/100.0);
                            }
                        }
                        points->InsertNextPoint(coord[0], coord[1], coord[2]);
                    }
                }
                else{
                    for(int i=0; i<3; i++){
                        PPL::Variable x(i);
                        if((int)g.space_dimension()>i){
                            coord.push_back(g.coefficient(x).get_d()/(g.divisor().get_d()*IBEX_PPL_PRECISION));
                        }
                        else{
                            coord.push_back(0.0);
                        }
                    }
                    //                 cout << "coord=" << coord[0] << ", " << coord[1] << ", " << coord[2] << endl;
                    points->InsertNextPoint(coord[0], coord[1], coord[2]);
                }
            }
        }
    }

    vtkSmartPointer< vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
    polydata->SetPoints(points);
    //polydata->SetVerts(vertices);

    // Create the convex hull of the pointcloud
    vtkSmartPointer<vtkDelaunay3D> delaunay = vtkSmartPointer< vtkDelaunay3D >::New();
    delaunay->SetInputData(polydata);
    delaunay->Update();

    vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
    surfaceFilter->SetInputConnection(delaunay->GetOutputPort());
    surfaceFilter->Update();

    vtkSmartPointer<vtkXMLPolyDataWriter> outputWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    string file = filename + ".vtp";
    outputWriter->SetFileName(file.c_str());
    outputWriter->SetInputData(surfaceFilter->GetOutput());
    outputWriter->Write();
}

void write_VTK(PPL::C_Polyhedron &ph, string filename){
    std::vector<PPL::C_Polyhedron> ph_list;
    ph_list.push_back(ph);
    write_VTK(ph_list, filename);
}

int test(){
    PPL::C_Polyhedron ph1(3);
    PPL::C_Polyhedron ph2(3);

#pragma omp parallel
    {
        PPL::Thread_Init *thread_init = nullptr;
        bool thread_init_valid = true;
#pragma omp master
{
    thread_init_valid = false;
}
    if(thread_init_valid)
        thread_init = new PPL::Thread_Init();
#pragma omp for
        for(int i=0; i<4; i++){
            PPL::C_Polyhedron ph3(3);
            ph3.intersection_assign(ph2);
        }
    if(thread_init_valid)
        delete(thread_init);
    }
    cout << omp_get_thread_num() << endl;
    return 0;
}

int test2(){
    PPL::C_Polyhedron ph3(3);
    PPL::C_Polyhedron ph4(3);
    ph3.intersection_assign(ph4);
    return 0;
}

int main(){
//    int dimension = 3;

//    IntervalVector vec(dimension);
//    vec[0] = ibex::Interval(-0.3, 0.3);
//    vec[1] = ibex::Interval(-0.3, 0.3);
//    vec[2] = ibex::Interval(1);

//    IntervalVector face_in(3);
//    face_in[0] = ibex::Interval(-1, 1);
//    face_in[1] = ibex::Interval(-1, 1);
//    face_in[2] = ibex::Interval(-1);
//    C_Polyhedron ph_in(iv_2_box(face_in));
//    C_Polyhedron ph_projection = add_ray(vec, ph_in);
//    IntervalVector face_out(3);
//    face_out[0] = ibex::Interval(-1, 1);
//    face_out[1] = ibex::Interval(-1, 1);
//    face_out[2] = ibex::Interval(1);
//    C_Polyhedron ph_out(iv_2_box(face_out));
//    test();
//    test2();
//    PPL::C_Polyhedron ph3(3);
//    PPL::C_Polyhedron ph4(3);
//    ph3.intersection_assign(ph4);

//    std::cout.precision(30);
//    mpq_class t(500.123540000000000);
//    cout << t.get_d() << endl;
//    cout << t.get_num().get_d() << endl;
//    cout << t.get_den().get_d() << endl;
//    cout << t.get_num().get_d() / t.get_den().get_d() << endl;
//    mpq_class t1(500.12354);
//    cout << t1.get_d() << endl;
//    cout << t1.get_num().get_d() << endl;
//    cout << t1.get_den().get_d() << endl;
//    cout << t.get_num().get_d() / t.get_den().get_d() << endl;


    //    cout << ph_out.space_dimension() << endl;
    //    cout << ph_out.affine_dimension() << endl;
    //    cout << ph_out.is_discrete() << endl;
    //    cout << ph_out.is_empty() << endl;
    //    cout << ph_out.is_topologically_closed() << endl;

    //    write_VTK(ph_in, "ph_in");
    //    write_VTK(ph_out, "ph_out");
    //    write_VTK(ph_projection, "ph_projection");


//    ibex::Variable x(2), a, b, c;
//    Function f= Function(x, a, b, c, a*pow(x[0], 2)+b*x[0]+c-x[1]);
//    IntervalVector test(5);
//    test[0] = ibex::Interval(-10, 10);
//    test[1] = ibex::Interval(-10, 10);
//    test[2] = ibex::Interval(1);
//    test[3] = ibex::Interval(1);
//    test[4] = ibex::Interval(1);
//    cout << "test = " << test << endl;
//    ibex::CtcFwdBwd ctc(f);
//    ctc.contract(test);
//    cout << "test = " << test << endl;

    invariant::ResultStorageIBEX r(2, 1);
    ibex::IntervalVector iv_test(2, 1);
    iv_test[0] = ibex::Interval(0, 1);
    iv_test[1] = ibex::Interval(2, 3);
    r.push_back_input(iv_test, 1, 0, 0, 1, 0);


    invariant::ResultStoragePPL r2(2, 1);
    Parma_Polyhedra_Library::C_Polyhedron p(2);
    cout << p.is_universe() << endl;
    r2.push_back_input(p, 1, 0, 0, 1, 0);
    cout << "test" << endl;

    return 0;
}


