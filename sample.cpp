#ifndef __PROGTEST__

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <variant>
#include <vector>

using State = unsigned int;
using Symbol = uint8_t;

struct NFA {
    std::set<State> m_States;
    std::set<Symbol> m_Alphabet;
    std::map<std::pair<State, Symbol>, std::set<State>> m_Transitions;
    State m_InitialState;
    std::set<State> m_FinalStates;
};

struct DFA {
    std::set<State> m_States;
    std::set<Symbol> m_Alphabet;
    std::map<std::pair<State, Symbol>, State> m_Transitions;
    State m_InitialState;
    std::set<State> m_FinalStates;
};

void print_nfa(const NFA& a);

#endif

// Find and return the biggest value of NFA states
//
State find_delta_state(const NFA& a) {
    State max_state = 0;
    for (auto i : a.m_States) {
        if (i > max_state) {
            max_state = i;
        }
    }

    return max_state + 1;
}

// Modify all states of NFA a by adding delta
void increase_states_by_delta(NFA& a, State delta) {
    // Modify set of states
    std::set<State> states;
    for (auto i : a.m_States) {
        states.insert(i + delta);
    }
    a.m_States = states;

    // Modify set of final states
    states.clear();
    for (auto i : a.m_FinalStates) {
        states.insert(i + delta);
    }
    a.m_FinalStates = states;

    // Modify initial state
    a.m_InitialState += delta;

    std::map<std::pair<State, Symbol>, State> transitions;
    for (auto j = a.m_Transitions.begin(); j != a.m_Transitions.end(); ++j) {
        State s = j->second;
        State s1 = j->first.first;
        std::cout << "key [" << j->first.first << ", " << j->first.second << "]" << j->second << "\n";
    }
}

// Unify two NFAs with epsilon transition
// Input:
//      a, b NFAs to unify
// Output:
//      NFA - L(NFA) = L(a) U L(b)
// Lecture 3, page 12
NFA unify_nfa(const NFA& a, const NFA& b) {
    NFA res;
    res.m_Alphabet = a.m_Alphabet;
    for (auto i : b.m_Alphabet) {
        res.m_Alphabet.insert(i);
    }

    std::cout << "Alphabet of input NFAs:\n";
    print_nfa(a);
    print_nfa(b);
    std::cout << "Alphabet of unified NFA:\n";
    print_nfa(res);
    
    NFA b1 = b;
    increase_states_by_delta(b1, find_delta_state(a));
    print_nfa(b1);

    return res;
}

DFA unify(const NFA& a, const NFA& b) {
    // Calculate union NFA
    NFA res = unify_nfa(a, b);
    DFA dfa;
    return dfa;
}

DFA intersect(const NFA& a, const NFA& b) {
    DFA dfa;
    return dfa;
}


#ifndef __PROGTEST__

// You may need to update this function or the sample data if your state naming strategy differs.
bool operator==(const DFA& a, const DFA& b)
{
    return std::tie(a.m_States, a.m_Alphabet, a.m_Transitions, a.m_InitialState, a.m_FinalStates) == std::tie(b.m_States, b.m_Alphabet, b.m_Transitions, b.m_InitialState, b.m_FinalStates);
}

void print_nfa(const NFA& a) {
    std::cout << "Alphabet:\n";
    for (auto i : a.m_Alphabet) {
        std::cout << i;
    }

    std::cout << "\n";

    std::cout << "States:\n";
    for (auto i : a.m_States) {
        std::cout << i;
    }

    std::cout << "\n";
}

int main()
{
    NFA a1{
        {0, 1, 2},
        {'a', 'b'},
        {
            {{0, 'a'}, {0, 1}},
            {{0, 'b'}, {0}},
            {{1, 'a'}, {2}},
        },
        0,
        {2},
    };
    NFA a2{
        {0, 1, 2}, // 3,4,5
        {'a', 'b'},
        {
            {{0, 'a'}, {1}},
            {{1, 'a'}, {2}},
            {{2, 'a'}, {2}},
            {{2, 'b'}, {2}},
        },
        0,
        {2},
    };
    DFA a{
        {0, 1, 2, 3, 4},
        {'a', 'b'},
        {
            {{0, 'a'}, {1}},
            {{1, 'a'}, {2}},
            {{2, 'a'}, {2}},
            {{2, 'b'}, {3}},
            {{3, 'a'}, {4}},
            {{3, 'b'}, {3}},
            {{4, 'a'}, {2}},
            {{4, 'b'}, {3}},
        },
        0,
        {2},
    };
 //   assert(intersect(a1, a2) == a);

    NFA b1{
        {0, 1, 2, 3, 4},
        {'a', 'b'},
        {
            {{0, 'a'}, {1}},
            {{0, 'b'}, {2}},
            {{2, 'a'}, {2, 3}},
            {{2, 'b'}, {2}},
            {{3, 'a'}, {4}},
        },
        0,
        {1, 4},
    };
    NFA b2{
        {0, 1, 2, 3, 4},
        {'a', 'b'},
        {
            {{0, 'b'}, {1}},
            {{1, 'a'}, {2}},
            {{2, 'b'}, {3}},
            {{3, 'a'}, {4}},
            {{4, 'a'}, {4}},
            {{4, 'b'}, {4}},
        },
        0,
        {4},
    };
    DFA b{
        {0, 1, 2, 3, 4, 5, 6, 7, 8},
        {'a', 'b'},
        {
            {{0, 'a'}, {1}},
            {{0, 'b'}, {2}},
            {{2, 'a'}, {3}},
            {{2, 'b'}, {4}},
            {{3, 'a'}, {5}},
            {{3, 'b'}, {6}},
            {{4, 'a'}, {7}},
            {{4, 'b'}, {4}},
            {{5, 'a'}, {5}},
            {{5, 'b'}, {4}},
            {{6, 'a'}, {8}},
            {{6, 'b'}, {4}},
            {{7, 'a'}, {5}},
            {{7, 'b'}, {4}},
            {{8, 'a'}, {8}},
            {{8, 'b'}, {8}},
        },
        0,
        {1, 5, 8},
    };
//    assert(unify(b1, b2) == b);
    unify(b1, b2);

    NFA c1{
        {0, 1, 2, 3, 4},
        {'a', 'b'},
        {
            {{0, 'a'}, {1}},
            {{0, 'b'}, {2}},
            {{2, 'a'}, {2, 3}},
            {{2, 'b'}, {2}},
            {{3, 'a'}, {4}},
        },
        0,
        {1, 4},
    };
    NFA c2{
        {0, 1, 2},
        {'a', 'b'},
        {
            {{0, 'a'}, {0}},
            {{0, 'b'}, {0, 1}},
            {{1, 'b'}, {2}},
        },
        0,
        {2},
    };
    DFA c{
        {0},
        {'a', 'b'},
        {},
        0,
        {},
    };
//    assert(intersect(c1, c2) == c);
    unify(c1, c2);

    NFA d1{
        {0, 1, 2, 3},
        {'i', 'k', 'q'},
        {
            {{0, 'i'}, {2}},
            {{0, 'k'}, {1, 2, 3}},
            {{0, 'q'}, {0, 3}},
            {{1, 'i'}, {1}},
            {{1, 'k'}, {0}},
            {{1, 'q'}, {1, 2, 3}},
            {{2, 'i'}, {0, 2}},
            {{3, 'i'}, {3}},
            {{3, 'k'}, {1, 2}},
        },
        0,
        {2, 3},
    };
    NFA d2{
        {0, 1, 2, 3},
        {'i', 'k'},
        {
            {{0, 'i'}, {3}},
            {{0, 'k'}, {1, 2, 3}},
            {{1, 'k'}, {2}},
            {{2, 'i'}, {0, 1, 3}},
            {{2, 'k'}, {0, 1}},
        },
        0,
        {2, 3},
    };
    DFA d{
        {0, 1, 2, 3},
        {'i', 'k', 'q'},
        {
            {{0, 'i'}, {1}},
            {{0, 'k'}, {2}},
            {{2, 'i'}, {3}},
            {{2, 'k'}, {2}},
            {{3, 'i'}, {1}},
            {{3, 'k'}, {2}},
        },
        0,
        {1, 2, 3},
    };
//    assert(intersect(d1, d2) == d);
    unify(d1, d2);
}
#endif
