#include "SimpleSpring.h"

void SimpleSpring::computeA()
{
    m_matrixA << -m_damping/m_mass, -m_stiffness/m_mass,
                 1.0              , 0.0                ;
    m_matrixChanged = true;
}

void SimpleSpring::computeB()
{
    m_vectorB << m_gravity, 0.0;
}
