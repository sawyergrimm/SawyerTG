#include <SuperPupUtilities/StateMachine.hpp>

#include <Canis/Debug.hpp>

#include <algorithm>
#include <utility>

namespace SuperPupUtilities
{
    State::State(std::string _name) : m_name(std::move(_name)) {}

    State::State(std::string _name, StateMachine& _stateMachine) :
        m_name(std::move(_name)),
        m_stateMachine(&_stateMachine) {}

    StateMachine::StateMachine(Canis::Entity& _entity) :
        Canis::ScriptableEntity(_entity) {}

    const std::string& State::GetName() const
    {
        return m_name;
    }

    StateMachine* State::GetStateMachine() const
    {
        return m_stateMachine;
    }

    void StateMachine::Destroy()
    {
        ClearStates();
    }

    void StateMachine::ClearStates()
    {
        if (m_currentState != nullptr)
            m_currentState->Exit();

        m_currentState = nullptr;

        for (State* state : m_states)
        {
            if (state != nullptr)
                state->m_stateMachine = nullptr;
        }

        m_states.clear();
    }

    void StateMachine::AddState(State& _state)
    {
        if (std::find(m_states.begin(), m_states.end(), &_state) != m_states.end())
            return;

        _state.m_stateMachine = this;
        m_states.push_back(&_state);
    }

    bool StateMachine::ChangeState(const std::string& _stateName)
    {
        for (State* state : m_states)
        {
            if (state == nullptr || state->GetName() != _stateName)
                continue;

            if (m_currentState == state)
                return true;

            if (m_currentState != nullptr)
                m_currentState->Exit();

            m_currentState = state;
            m_currentState->Enter();
            return true;
        }

        Canis::Debug::Warning("StateMachine: state '%s' was not found.", _stateName.c_str());
        return false;
    }

    void StateMachine::Update(float _dt)
    {
        if (m_currentState == nullptr)
            return;

        m_currentState->Update(_dt);
    }

    State* StateMachine::GetCurrentState() const
    {
        return m_currentState;
    }

    const std::string& StateMachine::GetCurrentStateName() const
    {
        static const std::string emptyStateName = "";

        if (m_currentState == nullptr)
            return emptyStateName;

        return m_currentState->GetName();
    }
}
