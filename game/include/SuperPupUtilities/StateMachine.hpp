#pragma once

#include <Canis/Entity.hpp>

#include <string>
#include <vector>

namespace SuperPupUtilities
{
    class StateMachine;

    class State
    {
    public:
        explicit State(std::string _name);
        State(std::string _name, StateMachine& _stateMachine);
        virtual ~State() = default;

        const std::string& GetName() const;
        StateMachine* GetStateMachine() const;

        virtual void Enter() {}
        virtual void Update(float _dt) = 0;
        virtual void Exit() {}

    protected:
        friend class StateMachine;

        std::string m_name = "";
        StateMachine* m_stateMachine = nullptr;
    };

    class StateMachine : public Canis::ScriptableEntity
    {
    public:
        explicit StateMachine(Canis::Entity& _entity);

        void Destroy() override;
        void ClearStates();
        void AddState(State& _state);
        bool ChangeState(const std::string& _stateName);
        void Update(float _dt) override;

        State* GetCurrentState() const;
        const std::string& GetCurrentStateName() const;

    private:
        std::vector<State*> m_states = {};
        State* m_currentState = nullptr;
    };
}
