#ifndef INTEGRATORS_H
#define INTEGRATORS_H

#include "Eigen/LU"

// --------------------------------------------------------------------------

template <typename S>
class OrdinaryDifferentialEquation
{
public:
    virtual S derivativeFunction(double t, const S &state) const = 0;
};

template <typename S, typename M>
class LinearODE : public OrdinaryDifferentialEquation<S>
{
public:
    // the derivative function should take a form f(t,y) = y' = Ay + b
    virtual const M &matrixA() const = 0;
    virtual const S &vectorB() const = 0;
    virtual bool matrixChanged() { return false; }
};

// --------------------------------------------------------------------------

template <typename S>
class Integrator
{
protected:
    OrdinaryDifferentialEquation<S> *m_ode;

    S       m_state;
    double  m_time;
    double  m_timeStep;

public:
    Integrator(OrdinaryDifferentialEquation<S> *ode, double dt)
        : m_ode(ode), m_time(0.0), m_timeStep(dt) {}

    void setState(const S &state)       { m_state = state; }
    S state() const                     { return m_state; }

    virtual void setTimeStep(double dt) { m_timeStep = dt; }
    double timeStep() const             { return m_timeStep; }

    virtual void step() = 0;
};

// --------------------------------------------------------------------------

template <typename S>
class ExplicitEulerIntegrator : public Integrator<S>
{
public:
    ExplicitEulerIntegrator(OrdinaryDifferentialEquation<S> *ode, double dt)
        : Integrator<S>(ode, dt)
    {}

    virtual void step()
    {
        double &t   = this->m_time;
        double &dt  = this->m_timeStep;
        S &y        = this->m_state;

        y += dt * this->m_ode->derivativeFunction(t, y);
        t += dt;
    }
};

// --------------------------------------------------------------------------

template <typename S, typename M>
class ImplicitEulerIntegrator : public Integrator<S>
{
protected:
    LinearODE<S, M>        *m_linearODE;
    Eigen::PartialPivLU<M>  m_factorized;

    // refactor method assumes the matrix type is an Eigen matrix
    void refactor()
    {
        M I         = M::Identity();
        double &dt  = this->m_timeStep;
        const M &A  = this->m_linearODE->matrixA();

        m_factorized = (I - dt * A).partialPivLu();
    }

public:
    ImplicitEulerIntegrator(LinearODE<S, M> *ode, double dt)
        : Integrator<S>(ode, dt), m_linearODE(ode)
    {}

    virtual void setTimeStep(double dt)
    {
        Integrator<S>::setTimeStep(dt);
        refactor();
    }

    virtual void step()
    {
        // if the linear ODE's matrix has changed, refactor our solution
        if (m_linearODE->matrixChanged()) refactor();

        double &t   = this->m_time;
        double &dt  = this->m_timeStep;
        S &y        = this->m_state;
        const S &b  = this->m_linearODE->vectorB();

        y = m_factorized.solve(y + dt * b);
        t += dt;
    }
};

// --------------------------------------------------------------------------

template <typename S>
class ModifiedMidpointIntegrator : public Integrator<S>
{
public:
    ModifiedMidpointIntegrator(OrdinaryDifferentialEquation<S> *ode, double dt)
        : Integrator<S>(ode, dt)
    {}

    virtual void step()
    {
        double &t   = this->m_time;
        double &dt  = this->m_timeStep;
        S &y        = this->m_state;

        // predictor step
        S yp = y + 0.5*dt * this->m_ode->derivativeFunction(t, y);

        // corrector step
        y += dt * this->m_ode->derivativeFunction(t + 0.5*dt, yp);

        t += dt;
    }
};

// --------------------------------------------------------------------------

template <typename S>
class RungeKutta4Integrator : public Integrator<S>
{
public:
    RungeKutta4Integrator(OrdinaryDifferentialEquation<S> *ode, double dt)
        : Integrator<S>(ode, dt)
    {}

    virtual void step()
    {
        double &t   = this->m_time;
        double &dt  = this->m_timeStep;
        S &y        = this->m_state;

        // calculate 4 Runge-Kutta steps
        S dy1 = dt * this->m_ode->derivativeFunction(t, y);
        S dy2 = dt * this->m_ode->derivativeFunction(t + 0.5*dt, y + 0.5*dy1);
        S dy3 = dt * this->m_ode->derivativeFunction(t + 0.5*dt, y + 0.5*dy2);
        S dy4 = dt * this->m_ode->derivativeFunction(t + dt, y + dy3);

        // perform state update
        y += 1.0/6.0 * (dy1 + 2.0*dy2 + 2.0*dy3 + dy4);

        t += dt;
    }
};

// --------------------------------------------------------------------------

#endif // INTEGRATORS_H
