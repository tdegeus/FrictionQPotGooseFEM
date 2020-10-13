/*

(c - MIT) T.W.J. de Geus (Tom) | www.geus.me | github.com/tdegeus/FrictionQPotFEM

*/

#ifndef FRICTIONQPOTFEM_UNIFORMSINGLELAYER2D_HPP
#define FRICTIONQPOTFEM_UNIFORMSINGLELAYER2D_HPP

#include "UniformSingleLayer2d.h"

namespace FrictionQPotFEM {
namespace UniformSingleLayer2d {

inline void HybridSystem::setGeometry(
    const xt::xtensor<double, 2>& coor,
    const xt::xtensor<size_t, 2>& conn,
    const xt::xtensor<size_t, 1>& dofs,
    const xt::xtensor<size_t, 1>& iip,
    const xt::xtensor<size_t, 1>& elem_elastic,
    const xt::xtensor<size_t, 1>& elem_plastic)
{
    m_coor = coor;
    m_conn = conn;
    m_dofs = dofs;
    m_iip = iip;
    m_elem_elas = elem_elastic;
    m_elem_plas = elem_plastic;

    m_nnode = m_coor.shape(0);
    m_ndim = m_coor.shape(1);
    m_nelem = m_conn.shape(0);
    m_nne = m_conn.shape(1);

    m_nelem_elas = m_elem_elas.size();
    m_nelem_plas = m_elem_plas.size();
    m_conn_elas = xt::view(m_conn, xt::keep(m_elem_elas), xt::all());
    m_conn_plas = xt::view(m_conn, xt::keep(m_elem_plas), xt::all());
    m_N = m_nelem_plas;

    m_vector = GF::VectorPartitioned(m_conn, m_dofs, m_iip);
    m_vector_elas = GF::VectorPartitioned(m_conn_elas, m_dofs, m_iip);
    m_vector_plas = GF::VectorPartitioned(m_conn_plas, m_dofs, m_iip);

    m_quad = QD::Quadrature(m_vector.AsElement(m_coor));
    m_quad_elas = QD::Quadrature(m_vector_elas.AsElement(m_coor));
    m_quad_plas = QD::Quadrature(m_vector_plas.AsElement(m_coor));
    m_nip = m_quad.nip();

    m_u = m_vector.AllocateNodevec(0.0);
    m_v = m_vector.AllocateNodevec(0.0);
    m_a = m_vector.AllocateNodevec(0.0);
    m_v_n = m_vector.AllocateNodevec(0.0);
    m_a_n = m_vector.AllocateNodevec(0.0);

    m_ue = m_vector.AllocateElemvec(0.0);
    m_fe = m_vector.AllocateElemvec(0.0);
    m_ue_plas = m_vector_plas.AllocateElemvec(0.0);
    m_fe_plas = m_vector_plas.AllocateElemvec(0.0);

    m_felas = m_vector.AllocateNodevec(0.0);
    m_fplas = m_vector.AllocateNodevec(0.0);
    m_fdamp = m_vector.AllocateNodevec(0.0);
    m_fint = m_vector.AllocateNodevec(0.0);
    m_fext = m_vector.AllocateNodevec(0.0);
    m_fres = m_vector.AllocateNodevec(0.0);

    m_Eps = m_quad.AllocateQtensor<2>(0.0);
    m_Sig = m_quad.AllocateQtensor<2>(0.0);
    m_Eps_elas = m_quad_elas.AllocateQtensor<2>(0.0);
    m_Sig_elas = m_quad_elas.AllocateQtensor<2>(0.0);
    m_Eps_plas = m_quad_plas.AllocateQtensor<2>(0.0);
    m_Sig_plas = m_quad_plas.AllocateQtensor<2>(0.0);

    m_M = GF::MatrixDiagonalPartitioned(m_conn, m_dofs, m_iip);
    m_D = GF::MatrixDiagonal(m_conn, m_dofs);

    m_material = GM::Array<2>({m_nelem, m_nip});
    m_material_elas = GM::Array<2>({m_nelem_elas, m_nip});
    m_material_plas = GM::Array<2>({m_nelem_plas, m_nip});
}

inline void HybridSystem::evalAllSet()
{
    m_allset = m_set_M && m_set_D && m_set_elas && m_set_plas && m_dt > 0.0;
}

inline void HybridSystem::initMaterial()
{
    if (!(m_set_elas && m_set_plas)) {
        return;
    }

    m_material.check();
    m_material_elas.check();
    m_material_plas.check();

    m_material.setStrain(m_Eps);
    m_material_elas.setStrain(m_Eps_elas);
    m_material_plas.setStrain(m_Eps_plas);

    m_K_elas = GF::Matrix(m_conn_elas, m_dofs);
    m_K_elas.assemble(m_quad_elas.Int_gradN_dot_tensor4_dot_gradNT_dV(m_material_elas.Tangent()));
}

template <class T>
inline void HybridSystem::setMatrix(T& matrix, const xt::xtensor<double, 1>& val_elem)
{
    FRICTIONQPOTFEM_ASSERT(val_elem.size() == m_nelem);

    QD::Quadrature nodalQuad(m_vector.AsElement(m_coor), QD::Nodal::xi(), QD::Nodal::w());

    xt::xtensor<double, 2> val_quad = xt::empty<double>({m_nelem, nodalQuad.nip()});
    for (size_t q = 0; q < nodalQuad.nip(); ++q) {
        xt::view(val_quad, xt::all(), q) = val_elem;
    }

    matrix.assemble(nodalQuad.Int_N_scalar_NT_dV(val_quad));
}

inline void HybridSystem::setMassMatrix(const xt::xtensor<double, 1>& val_elem)
{
    this->setMatrix(m_M, val_elem);
    m_set_M = true;
    this->evalAllSet();
}

inline void HybridSystem::setDampingMatrix(const xt::xtensor<double, 1>& val_elem)
{
    this->setMatrix(m_M, val_elem);
    m_set_D = true;
    this->evalAllSet();
}

inline void HybridSystem::setElastic(
    const xt::xtensor<double, 1>& K_elem,
    const xt::xtensor<double, 1>& G_elem)
{
    xt::xtensor<size_t, 2> I = xt::zeros<size_t>({m_nelem, m_nip});
    xt::xtensor<size_t, 2> idx = xt::zeros<size_t>({m_nelem, m_nip});
    xt::view(I, xt::keep(m_elem_elas), xt::all()) = 1ul;
    xt::view(idx, xt::keep(m_elem_elas), xt::all()) = xt::arange<size_t>(m_nelem_elas).reshape({-1, 1});
    m_material.setElastic(I, idx, K_elem, G_elem);

    I = xt::ones<size_t>({m_nelem_elas, m_nip});
    idx = xt::zeros<size_t>({m_nelem_elas, m_nip});
    xt::view(idx, xt::range(0, m_nelem_elas), xt::all()) = xt::arange<size_t>(m_nelem_elas).reshape({-1, 1});
    m_material_elas.setElastic(I, idx, K_elem, G_elem);

    m_set_elas = true;
    this->evalAllSet();
    this->initMaterial();
}

inline void HybridSystem::setPlastic(
    const xt::xtensor<double, 1>& K_elem,
    const xt::xtensor<double, 1>& G_elem,
    const xt::xtensor<double, 2>& epsy_elem)
{
    xt::xtensor<size_t, 2> I = xt::zeros<size_t>({m_nelem, m_nip});
    xt::xtensor<size_t, 2> idx = xt::zeros<size_t>({m_nelem, m_nip});
    xt::view(I, xt::keep(m_elem_plas), xt::all()) = 1ul;
    xt::view(idx, xt::keep(m_elem_plas), xt::all()) = xt::arange<size_t>(m_nelem_plas).reshape({-1, 1});
    m_material.setCusp(I, idx, K_elem, G_elem, epsy_elem);

    I = xt::ones<size_t>({m_nelem_plas, m_nip});
    idx = xt::zeros<size_t>({m_nelem_plas, m_nip});
    xt::view(idx, xt::range(0, m_nelem_plas), xt::all()) = xt::arange<size_t>(m_nelem_plas).reshape({-1, 1});
    m_material_plas.setCusp(I, idx, K_elem, G_elem, epsy_elem);

    m_set_plas = true;
    this->evalAllSet();
    this->initMaterial();
}

inline void HybridSystem::setDt(double dt)
{
    m_dt = dt;
    this->evalAllSet();
}

inline auto HybridSystem::nelem() const
{
    return m_nelem;
}

inline void HybridSystem::computeStress()
{
    m_vector.asElement(m_u, m_ue);
    m_quad.symGradN_vector(m_ue, m_Eps);
    m_material.setStrain(m_Eps);
    m_material.stress(m_Sig);
}

inline void HybridSystem::computeStressPlastic()
{
    m_vector_plas.asElement(m_u, m_ue_plas);
    m_quad_plas.symGradN_vector(m_ue_plas, m_Eps_plas);
    m_material_plas.setStrain(m_Eps_plas);
    m_material_plas.stress(m_Sig_plas);
    m_quad_plas.int_gradN_dot_tensor2_dV(m_Sig_plas, m_fe_plas);
    m_vector_plas.assembleNode(m_fe_plas, m_fplas);
    m_K_elas.dot(m_u, m_felas);
}

inline void HybridSystem::timeStep()
{
    // history

    m_t += m_dt;

    xt::noalias(m_v_n) = m_v;
    xt::noalias(m_a_n) = m_a;

    // new displacement

    xt::noalias(m_u) = m_u + m_dt * m_v + 0.5 * std::pow(m_dt, 2.0) * m_a;

    // compute strain/strain, and corresponding force

    computeStressPlastic();

    // estimate new velocity, update corresponding force

    xt::noalias(m_v) = m_v_n + m_dt * m_a_n;

    m_D.dot(m_v, m_fdamp);

    // compute residual force & solve

    xt::noalias(m_fint) = m_felas + m_fplas + m_fdamp;

    m_vector.copy_p(m_fint, m_fext);

    xt::noalias(m_fres) = m_fext - m_fint;

    m_M.solve(m_fres, m_a);

    // re-estimate new velocity, update corresponding force

    xt::noalias(m_v) = m_v_n + 0.5 * m_dt * (m_a_n + m_a);

    m_D.dot(m_v, m_fdamp);

    // compute residual force & solve

    xt::noalias(m_fint) = m_felas + m_fplas + m_fdamp;

    m_vector.copy_p(m_fint, m_fext);

    xt::noalias(m_fres) = m_fext - m_fint;

    m_M.solve(m_fres, m_a);

    // new velocity, update corresponding force

    xt::noalias(m_v) = m_v_n + 0.5 * m_dt * (m_a_n + m_a);

    m_D.dot(m_v, m_fdamp);

    // compute residual force & solve

    xt::noalias(m_fint) = m_felas + m_fplas + m_fdamp;

    m_vector.copy_p(m_fint, m_fext);

    xt::noalias(m_fres) = m_fext - m_fint;

    m_M.solve(m_fres, m_a);
}


} // namespace UniformSingleLayer2d
} // namespace FrictionQPotFEM

#endif
