#ifndef SIMPLESPRING_H
#define SIMPLESPRING_H

#include "Eigen/Core"
#include "Integrators.h"

class SimpleSpring : public LinearODE<Eigen::Vector2d, Eigen::Matrix2d>
{
    double m_mass;
    double m_stiffness;
    double m_damping;
    double m_gravity;

    double m_initialPosition;
    double m_timeRemainder;

    // time integrator class
    Integrator<Eigen::Vector2d> *m_integrator;

    // derivative function f(t,y) = Ay + b
    Eigen::Matrix2d m_matrixA;
    Eigen::Vector2d m_vectorB;

    bool m_matrixChanged;

    void computeA();
    void computeB();

public:
    typedef Eigen::Vector2d StateType;
    typedef Eigen::Matrix2d MatrixType;

    SimpleSpring(double m = 1.0, double k = 1000.0, double b = 0.0, double g = -9.81)
        : m_mass(m), m_stiffness(k), m_damping(b), m_gravity(g), m_integrator(0),
          m_initialPosition(0), m_timeRemainder(0)
    {
        computeA();
        computeB();
    }

    ~SimpleSpring()
    {
        if (m_integrator) delete m_integrator;
    }

    void setIntegrator(Integrator<StateType> *i) { m_integrator = i; }

    void setMass(double m)      { m_mass = m;       computeA(); }
    void setStiffness(double k) { m_stiffness = k;  computeA(); }
    void setDamping(double b)   { m_damping = b;    computeA(); }
    void setGravity(double g)   { m_gravity = g;    computeB(); }

    void setTimeStep(double dt) { if (m_integrator) m_integrator->setTimeStep(dt); }
    void setInitialPosition(double p) { m_initialPosition = p; }

    void update(double elapsedTime = -1.0)
    {
        if (m_integrator)
        {
            double dt = m_integrator->timeStep();
            m_timeRemainder += (elapsedTime < 0.0 ? dt : elapsedTime);
            while (m_timeRemainder >= dt) {
                m_integrator->step();
                m_timeRemainder -= dt;
            }
        }
    }

    void reset()
    {
        Eigen::Vector2d initial(0.0, m_initialPosition);
        if (m_integrator) m_integrator->setState(initial);
    }

    // derivate function for this ODE: y' = f(t, y)
    virtual Eigen::Vector2d derivativeFunction(double t, const Eigen::Vector2d &y) const
    {
        return m_matrixA * y + m_vectorB;
    }

    virtual const Eigen::Matrix2d &matrixA() const { return m_matrixA; }
    virtual const Eigen::Vector2d &vectorB() const { return m_vectorB; }
    virtual bool matrixChanged()
                { return m_matrixChanged ? !(m_matrixChanged = false) : false; }

    Eigen::Vector2d currentState() const
    {
        if (m_integrator)   return m_integrator->state();
        else                return Eigen::Vector2d(0.0, 0.0);
    }
};

#endif // SIMPLESPRING_H
