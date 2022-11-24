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

void print_nfa(std::string text, const NFA& a);
void print_dfa(std::string text, const DFA& a);

#endif

// Find and return the biggest value of NFA states
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

    // Modify transition function
    std::map<std::pair<State, Symbol>, std::set<State>> transitions;
    for (auto j : a.m_Transitions) {
        std::pair<State, Symbol> key = j.first;
        key.first += delta;
        std::set<State> value;
        for (auto k : j.second) {
            value.insert(k + delta);
        }
        transitions.insert({ key, value });
    }

    a.m_Transitions = transitions;
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
    
    // This is to gurantee NFAs don't have common states
    NFA b1 = b;
    increase_states_by_delta(b1, find_delta_state(a));
    // 1. Q <- Q1 U Q2 U {q0}
    res.m_States = a.m_States;
    for (auto i : b1.m_States) {
        res.m_States.insert(i);
    }
    res.m_InitialState = find_delta_state(b1);
    res.m_States.insert(res.m_InitialState);

    // Compose transition function of NFA res
    std::map<std::pair<State, Symbol>, std::set<State>> transitions;
    
    // 3. delta(q, a) <- delta1(q, a)
    transitions = a.m_Transitions;
    
    // 4. delta(q, a) <- delta2(q, a)
    for (auto i : b1.m_Transitions) {
        transitions.insert(i);
    }

    // 2.   delta(q0, epsilon) <- {q01, q02}
    std::pair<State, Symbol> key = {res.m_InitialState, '\0'};
    std::set<State> value = { a.m_InitialState, b1.m_InitialState };
    transitions.insert({ key, value });
    res.m_Transitions = transitions;

    // 5. F <- F1 U F2
    res.m_FinalStates = a.m_FinalStates;
    for (auto i : b1.m_FinalStates) {
        res.m_FinalStates.insert(i);
    }
    
    print_nfa("NFA a", a);
    print_nfa("NFA b", b1);
    print_nfa("Unified NFA", res);
    
    return res;
}

// Calculates epsilon closure for a state \s of NFA \a by definition lecture 3 p. 25
std::set<State> e_closure(const NFA& a, State s) {
    std::set<State> res = { s };
    int cnt = 1;
    while (1) {
        for (auto i : res) {
            std::pair<State, Symbol> key = { i, '\0' };
            auto pos = a.m_Transitions.find(key);
            if (pos == a.m_Transitions.end()) {
                // Epsilon transition for this state not found
                continue;
            }
            // pos->second is set of states
            for (auto j : pos->second) {
                res.insert(j);
            }
        }
        if (res.size() == cnt) {
            break;
        }

        cnt = res.size();
    }

    std::cout << "Epsilon closure for state " << s << "\n";
    for (auto k : res) {
        std::cout << k << " ";
    }
    std::cout << "\n";
    
    return res;
}

bool is_set_intersect_empty(const std::set<State>& a, const std::set<State>& b) {
    for (auto i : a) {
        for (auto j : b) {
            if (i == j) {
                return false;
            }
        }
    }

    return true;
}

// Conversion NFA with epsilon transitions into NFA without epsilon transitions
NFA e_transition_removal(const NFA& a) {
    NFA res;
    
    res.m_States = a.m_States;
    res.m_Alphabet = a.m_Alphabet;
    res.m_InitialState = a.m_InitialState;
    
    // Compose delta'(transition function on NFA res)
    std::map<std::pair<State, Symbol>, std::set<State>> transitions;
    for (auto state : res.m_States) {
        std::set<State> e_clos = e_closure(a, state);
        for (auto symbol : res.m_Alphabet) {
            std::pair<State, Symbol> key = { state, symbol };
            std::set<State> value;
            for (auto clos_state : e_clos) {
                std::pair<State, Symbol> clos_key = { clos_state, symbol };
                auto pos = a.m_Transitions.find(clos_key);
                if (pos == a.m_Transitions.end()) {
                    // Not found
                    continue;
                }
                // pos->second is set of states
                for (auto j : pos->second) {
                    value.insert(j);
                }
            }
            transitions.insert({ key, value });
        }
    }
    res.m_Transitions = transitions;

    // Compose F'(final states of NFA res)
    std::set<State> fin_states;
    for (auto q : res.m_States) {
        std::set<State> e_clos = e_closure(a, q);
        if (!is_set_intersect_empty(e_clos, a.m_FinalStates)) {
            fin_states.insert(q);
        }
    }

    res.m_FinalStates = fin_states;

    return res;
}

DFA unify(const NFA& a, const NFA& b) {
    // Calculate union NFA with Epsilon closure
    NFA res = unify_nfa(a, b);
    DFA dfa;
    for (auto i : res.m_States) {
        e_closure(res, i);
    }
    
    return dfa;
}

DFA intersect(const NFA& a, const NFA& b) {
    DFA dfa;
    return dfa;
}

typedef std::set<State> Combined_state;

// Combined states needs to be mapped into states.
// This map is filled during determinization of combined states of DFA.
std::map<Combined_state, State> combined_states;
State dfa_state = 1;
// Return value:
//      true  if combined_state is added
//      false otherwise
bool store_combined_state(Combined_state state) {
    // Check  that \a states is in combined_states
    auto pos = combined_states.find(state);
    if (pos != combined_states.end()) {
        return false;
    }
    combined_states.insert({ state, dfa_state });
    dfa_state++;

    return true;
}

void print_comb_state(Combined_state state) {
    std::cout << "{ ";
    for (auto j : state) {
        std::cout << j << ", ";
    }
    std::cout << "}";
}

// NFA to DFA conversion using subset construction
// Input:
//      NFA    
//  Output:
//      DFA
//  Lecture 3, p. 3 (NFA determinization)
DFA nfa_determinization(const NFA& a) {
    DFA res;
    std::set<Combined_state> Q;
    std::map<std::pair<Combined_state, Symbol>, Combined_state> transitions;
    std::set<Combined_state> tmp;
    tmp.insert({ a.m_InitialState });
    store_combined_state({ a.m_InitialState });
    // Determine set of states and transition function of DFA
    while (1) {
        if (tmp.empty()) {
            break;
        }
        std::set<Combined_state> tmp2;
        for (auto comb_state : tmp) {
            for (auto symbol : a.m_Alphabet) {
                Combined_state uni;
                for (auto state : comb_state) {
                    auto pos = a.m_Transitions.find({ state, symbol });
                    if (pos != a.m_Transitions.end()) {
                        // Add all states from pos.second
                        for (auto s : pos->second) {
                            uni.insert(s);
                        }

                    }
                }
                // Try to save new combined state
                if (store_combined_state(uni)) {
                    tmp2.insert(uni);
                }
                transitions.insert({{ comb_state, symbol }, uni});
            }
            Q.insert(comb_state);
        }

        tmp = tmp2;
        // Print transitions
        std::cout << "{\n";
        for (auto i : transitions) {
            auto value = i.second;
            auto cs = i.first.first;
            auto sym = i.first.second;
            std::cout << "{ ";
            print_comb_state(cs);
            std::cout << ", " << sym << "} -> ";
            print_comb_state(value);
            std::cout << "\n";
        }
        std::cout << "}\n";
        
        // Print states of DFA
        std::cout << "{\n";
        for (auto i : Q) {
            std::cout << "  ";
            print_comb_state(i);
            std::cout << "\n";
        }
        std::cout << "}\n";
    }
    
    // Determine set of final states
    std::set<Combined_state> F;
    for (auto comb_state : Q) {
        for (auto state : comb_state) {
            auto pos = a.m_FinalStates.find(state);
            if (pos != a.m_FinalStates.end()) {
                F.insert(comb_state);
                break;
            }
        }
    }

    // Build DFA
    // res.m_States
    for (auto comb_state : Q) {
        auto pos = combined_states.find(comb_state);
        if (pos == combined_states.end()) {
            std::cerr << "Combined state not found";
            exit(1);
        }
        res.m_States.insert(pos->second);
    }

    // res.m_Alphabet
    res.m_Alphabet = a.m_Alphabet;

    // res.m_InitialState
    auto pos = combined_states.find({ a.m_InitialState });
    if (pos == combined_states.end()) {
        std::cerr << "Initial combined state not found";
        exit(1);
    }
    res.m_InitialState = pos->second;

    // res.m_FinalStates
    for (auto comb_state : F) {
        auto pos = combined_states.find(comb_state);
        if (pos == combined_states.end()) {
            std::cerr << "Combined state not found";
            exit(1);
        }
        res.m_FinalStates.insert(pos->second);
    }

    // res.m_Transition
    for (auto i : transitions) {
        auto key = i.first;
        auto value = i.second;
        //  TODO: pos must be found!
        auto pos = combined_states.find(key.first);
        auto pos2 = combined_states.find(value);
        res.m_Transitions.insert({{ pos->second, key.second }, pos2->second });
    }
/*    std::cout << "Final states:\n";
    for (auto i : F) {
        print_comb_state(i);
        std::cout << "\n";
    }

    std::cout << "Mapping from combined states to states\n";
    for (auto j : combined_states) {
        print_comb_state(j.first);
        std::cout << " -> " << j.second << "\n";
    }
*/
    print_nfa("Print ", a);
    print_dfa("Print ", res);
    
    return res;
}


#ifndef __PROGTEST__

// You may need to update this function or the sample data if your state naming strategy differs.
bool operator==(const DFA& a, const DFA& b)
{
    return std::tie(a.m_States, a.m_Alphabet, a.m_Transitions, a.m_InitialState, a.m_FinalStates) == std::tie(b.m_States, b.m_Alphabet, b.m_Transitions, b.m_InitialState, b.m_FinalStates);
}

void print_nfa(std::string text, const NFA& a) {
    std::cout << "NFA: " << text << "\n";
    std::cout << "\tAlphabet [";
    for (auto i : a.m_Alphabet) {
        std::cout << " '" << i << "'";
    }
    std::cout << " ]\n";

    std::cout << "\tStates [";
    for (auto i : a.m_States) {
        std::cout << " " << i;
    }
    std::cout << " ]\n";

    std::cout << "\tTransitions : \n";
    for (auto j : a.m_Transitions) {
        std::cout << "\t\tkey [ " << j.first.first << ", '" << j.first.second << "' ], value [";
        for (auto k : j.second) {
            std::cout << " " << k;
        }
        std::cout << " ]\n";
    }

    std::cout << "\tFinal States [";
    for (auto i : a.m_FinalStates) {
        std::cout << " " << i;
    }
    std::cout << " ]\n";

    std::cout << "\tInitial State [ " << a.m_InitialState << " ]\n";
}

void print_dfa(std::string text, const DFA& a) {
    std::cout << "DFA: " << text << "\n";
    std::cout << "\tAlphabet [";
    for (auto i : a.m_Alphabet) {
        std::cout << " '" << i << "'";
    }
    std::cout << " ]\n";

    std::cout << "\tStates [";
    for (auto i : a.m_States) {
        std::cout << " " << i;
    }
    std::cout << " ]\n";

    std::cout << "\tTransitions : \n";
    for (auto j : a.m_Transitions) {
        std::cout << "\t\tkey [ " << j.first.first << ", '" << j.first.second <<
            "' ], value [ " << j.second << " ]\n";
    }

    std::cout << "\tFinal States [";
    for (auto i : a.m_FinalStates) {
        std::cout << " " << i;
    }
    std::cout << " ]\n";

    std::cout << "\tInitial State [ " << a.m_InitialState << " ]\n";
}

int main()
{
    // Automat from lecture 3, p. 4
    NFA test1 {
        {1, 2, 3, 4}, // q, q0, q1, qf
        {'0', '1'},
        {
            {{1, '0'}, {1, 2}},
            {{1, '1'}, {1, 3}},
            {{2, '0'}, {2, 4}},
            {{2, '1'}, {2}},
            {{3, '0'}, {3}},
            {{3, '1'}, {3, 4}},
        },
        1,
        {4},
    };
    nfa_determinization(test1);
    return 0;

    // Automat from lecture 3, p. 23
    NFA test {
        {0, 1, 2},
        {'a', 'b', 'c'},
        {
            {{0, 'a'}, {0}},
            {{0, '\0'}, {1}},
            {{1, 'b'}, {1}},
            {{1, '\0'}, {2}},
            {{2, 'c'}, {2}},
        },
        0,
        {2},
    };

    print_nfa("Orig\n", test);
    print_nfa("Epsilon transition removal\n", e_transition_removal(test));
    
    return 0;

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
    return 0;

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
