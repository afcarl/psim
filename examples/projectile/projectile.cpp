// What is the optimal angle at which to shoot off a point mass to get it to
// go as far as possible? (45 degrees)

#include <iostream>

#include <PSim/PSim.h>
#include <OpenSim/OpenSim.h>
#include <OpenSim/Simulation/SimbodyEngine/PlanarJoint.h>

// Parameters.
// ===========
// Angle of the projectile's initial velocity with the horizontal, in degrees.
class Angle : public PSim::Parameter {
OpenSim_DECLARE_CONCRETE_OBJECT(Angle, PSim::Parameter);
    virtual void apply(const double param,
            Model& model, SimTK::State& initState) const {

        const double angle = param * SimTK::Pi / 180.0;

        const double v = 3;

        const double vx = v * cos(angle);
        const double vy = v * sin(angle);

        const Coordinate& cx = model.getCoordinateSet().get("x");
        cx.setSpeedValue(initState, vx);

        const Coordinate& cy = model.getCoordinateSet().get("y");
        cy.setSpeedValue(initState, vy);
    }
};

// Objectives.
// ===========
class Range : public PSim::Objective {
OpenSim_DECLARE_CONCRETE_OBJECT(Range, PSim::Objective);
    SimTK::Real evaluate(const PSim::ParameterValueSet& pvalset,
            const Model& model,
            const PSim::StateTrajectory& states) const override
    {
        const Coordinate& c = model.getCoordinateSet().get("x");
        return -c.getValue(states.back());
    }
};

class Test : public PSim::IntegratingObjective {
OpenSim_DECLARE_CONCRETE_OBJECT(Test, PSim::IntegratingObjective);
    SimTK::Real derivative(const SimTK::State& s) const override {
        return 1;
    }
    void realizeReport(const SimTK::State& s) const { 
        // TODO std::cout << "DEBUG report" << std::endl;
    }
};

class MaxHeight : public PSim::Objective {
OpenSim_DECLARE_CONCRETE_OBJECT(MaxHeight, PSim::Objective);
public:

    class Max : public PSim::Maximum {
    OpenSim_DECLARE_CONCRETE_OBJECT(Max, PSim::Maximum);
    public:
        Max(const MaxHeight* mh) : mh(mh) {}
        double getInputVirtual(const SimTK::State& s) const override {
            return mh->height(s);
        }
        SimTK::Stage getDependsOnStageVirtual() const override {
            return SimTK::Stage::Position;
        }
    private:
        const MaxHeight* mh;
    };

    MaxHeight() : m_max(this) {
        //constructInfrastructure();
    }
    MaxHeight(const MaxHeight& mh) : PSim::Objective(mh), m_max(this) {}
    SimTK::Real evaluate(const PSim::ParameterValueSet& pvalset,
            const Model& model,
            const PSim::StateTrajectory& states) const override {
        std::cout << m_max.maximum(states.back()) << std::endl;
        return m_max.maximum(states.back());
    }
    double height(const SimTK::State& s) const {
        return getModel().getCoordinateSet().get("y").getValue(s);
    }
private:
    /*
    // TODO should not be necessary.
    void constructProperties() override {}
    void constructOutputs() override {
        constructOutput<double>("height", &MaxHeight::height,
                SimTK::Stage::Position);
    }
    */

    void connectToModel(Model& model) override {
        Super::connectToModel(model);
        addComponent(&m_max);
        // m_max.getInput("input").connect(getOutput("height"));
    }
    Max m_max;
};


// Event handlers.
// ===============
namespace OpenSim {

class TriggeredEventHandler : public ModelComponent
{
OpenSim_DECLARE_ABSTRACT_OBJECT(TriggeredEventHandler, ModelComponent);
public:
    OpenSim_DECLARE_PROPERTY(required_stage, int,
            "The stage at which the event occurs.");

    TriggeredEventHandler() {
        constructProperties();
    }

    virtual void handleEvent(SimTK::State& state, SimTK::Real accuracy,
            bool& shouldTerminate) const = 0;
    virtual SimTK::Real getValue(const SimTK::State& s) const = 0;

private:

    void constructProperties() {
        constructProperty_required_stage(SimTK::Stage::Dynamics);
    }

    void addToSystem(SimTK::MultibodySystem& system) const override {
        // TODO is this okay for memory?
        system.addEventHandler(
                new SimbodyHandler(SimTK::Stage(get_required_stage()), this));
    }

    class SimbodyHandler : public SimTK::TriggeredEventHandler {
    public:
         SimbodyHandler(SimTK::Stage requiredStage,
                 const OpenSim::TriggeredEventHandler* handler) :
             SimTK::TriggeredEventHandler(requiredStage), m_handler(handler) {}
        void handleEvent(SimTK::State& state, SimTK::Real accuracy,
                bool& shouldTerminate) const override
        { m_handler->handleEvent(state, accuracy, shouldTerminate); }
        SimTK::Real getValue(const SimTK::State& s) const override
        { return m_handler->getValue(s); }
    private:
        const OpenSim::TriggeredEventHandler* m_handler;
    };
};

} // namespace OpenSim

// End the simulation when y = 0.
class HitGround : public TriggeredEventHandler {
OpenSim_DECLARE_CONCRETE_OBJECT(HitGround, TriggeredEventHandler);
public:
    void handleEvent(SimTK::State& state, SimTK::Real accuracy,
            bool& shouldTerminate) const override
    { shouldTerminate = true; }
    SimTK::Real getValue(const SimTK::State& s) const override {
        const Coordinate& c = getModel().getCoordinateSet().get("y");
        return c.getValue(s);
    }
};

Model createModel()
{
    Model model;

    Body* body = new Body("projectile", 1, SimTK::Vec3(0), SimTK::Inertia(1));
    model.addBody(body);

    Joint* joint = new PlanarJoint("joint",
            model.getGroundBody(), SimTK::Vec3(0), SimTK::Vec3(0),
            *body, SimTK::Vec3(0), SimTK::Vec3(0));
    joint->getCoordinateSet().get(0).setName("theta");
    joint->getCoordinateSet().get(1).setName("x");
    joint->getCoordinateSet().get(2).setName("y");
    model.addJoint(joint);

    model.addModelComponent(new HitGround());

    return model;
}

int main()
{
    // Set up tool.
    // ============
    PSim::Tool pstool;
    pstool.set_visualize(true);
    pstool.set_optimization_convergence_tolerance(1e-10);

    pstool.set_model(createModel());

    // Set up parameters.
    // ==================
    Angle angle;
    angle.setName("angle");
    angle.set_default_value(80);
    angle.set_lower_limit(1);
    angle.set_upper_limit(90);
    angle.set_lower_opt(0);
    angle.set_upper_opt(90);
    pstool.append_parameters(angle);

    // Set up objectives.
    // ==================
//    Range range;
//    range.set_weight(1);
//    pstool.append_objectives(range);
//    Test integratingObj;
//    integratingObj.set_weight(0);
//    pstool.append_objectives(integratingObj);
    MaxHeight maxHeight;
    maxHeight.set_weight(1);
    pstool.append_objectives(maxHeight);

    // Wrap up.
    // ========
    pstool.setSerializeAllDefaults(true);
    pstool.print("PSimToolSetup.xml");

    PSim::ParameterValueSet soln = pstool.run();
    soln.print("solution.xml");

    return 0;
}
