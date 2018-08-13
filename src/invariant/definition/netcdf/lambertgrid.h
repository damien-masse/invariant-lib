#ifndef LAMBERTGRID_H
#define LAMBERTGRID_H

#include <string>
#include <vector>
#include "dataSet/datasetnode.h"
#include "dataSet/datasetvirtualnode.h"
#include <netcdf>

namespace invariant {

class LambertGrid
{
public:
    /**
     * @brief LambertGrid
     * @param file_name
     */
    LambertGrid(const std::string& file_name);

    /**
     * @brief destructor LambertGrid
     */
    ~LambertGrid();

    /**
     * @brief get_U_X
     * @return
     */
    const std::vector<std::vector<double>>& get_U_X() const;

    /**
     * @brief get_U_Y
     * @return
     */
    const std::vector<std::vector<double>>& get_U_Y() const;

    /**
     * @brief get_V_X
     * @return
     */
    const std::vector<std::vector<double>>& get_V_X() const;

    /**
     * @brief get_V_Y
     * @return
     */
    const std::vector<std::vector<double>>& get_V_Y() const;

    /**
     * @brief eval
     */
    void eval(const double &x, const double &y, const double &t, double &u, double &v) const;

private:
    /**
     * @brief compute_grid_proj
     * @param dataFile
     */
    void compute_grid_proj(netCDF::NcFile &dataFile);

    /**
     * @brief fill_leaf
     * @param leaf_list
     * @param X
     * @param Y
     */
    void fill_leaf(std::vector<std::pair<DataSetVirtualNode*, std::vector<std::array<int, 2>>>> &leaf_list,
                   std::vector<std::vector<double>> &X,
                   std::vector<std::vector<double>> &Y);

private:
    std::vector<std::vector<double>> m_U_X, m_U_Y, m_V_X, m_V_Y;

    DataSetVirtualNode *m_dataSet_U = nullptr;
    DataSetVirtualNode *m_dataSet_V = nullptr;

    std::vector<std::array<int, 2>> m_position_U, m_position_V;

    double m_U_scale_factor, m_U_add_offset;
    double m_V_scale_factor, m_V_add_offset;
    short int m_U_Fill_Value, m_V_Fill_Value;

    std::vector<std::vector<std::vector<short int>>> m_U, m_V;
    std::vector<double> m_time;
};

inline const std::vector<std::vector<double>>& LambertGrid::get_U_X() const{
    return m_U_X;
}

inline const std::vector<std::vector<double>>& LambertGrid::get_U_Y() const{
    return m_U_Y;
}

inline const std::vector<std::vector<double>>& LambertGrid::get_V_X() const{
    return m_V_X;
}

inline const std::vector<std::vector<double>>& LambertGrid::get_V_Y() const{
    return m_V_Y;
}

}

#endif // LAMBERTGRID_H
