#include <bits/stdc++.h>
using namespace std;

// g++ -std=c++17 -Wall -o main main.cpp; ./main

struct TransitionResult {
    string next_state;
    char write;
    char direction;
};

int main() {
    bool repeat=true;
    while (repeat) {

        // Construccion de tabla
        map<pair<string, char>, TransitionResult> charter;
        string x, initial_state, final_state, word, curr_state, temp, input;

        cout << "Ingrese estado inicial: "; getline(cin, initial_state);
        cout << "Ingrese (estado),(letra) (n=terminar): ";
        while (getline(cin, input) && input != "n") {
            TransitionResult mod;
            pair<string, char> transition;

            stringstream ss(input);
            getline(ss, transition.first, ',');
            getline(ss, temp, ','); transition.second = temp[0];

            cout << "Ingrese modificacion (ej:q1,s,D): ";
            getline(cin, input);
            stringstream ss_val(input);
            getline(ss_val, mod.next_state, ',');
            getline(ss_val, temp, ','); mod.write = temp[0];
            getline(ss_val, temp, ','); mod.direction = temp[0];
            charter[transition] = mod;

            cout << "Ingrese (estado),(letra) (n=terminar): ";
        }
        cout << "Ingrese estado final: "; getline(cin, final_state);

        // ----------------------------------------------------------------------------
        cout << "\n--- contenido automata ---\n";
        for (const auto& [key,value] : charter) {
            string current_state = key.first;
            char read_char = key.second;
            string next_state = value.next_state;
            char write_char = value.write;
            char direction = value.direction;

            cout << "Origen: (" << current_state << ", " << read_char << ") "
            << "-> Destino: (" << next_state << ", " << write_char << ", " << direction << ")\n";
        }
        cout << "-------------------------------\n";
        // ----------------------------------------------------------------------------

        // Loop de palabras
        cout << "Ingrese palabra (n=terminar): ";
        while (getline(cin, word) && word != "n") {
            bool accepted = true;
            curr_state = initial_state;
            size_t curr_char = 0;

            while (curr_state != final_state) {

                cout << "Curr_state: " << curr_state << "   curr_char: " << word[curr_char] << "    Word: " << word << endl;

                auto it = charter.find({curr_state, word[curr_char]});
                if (it == charter.end()) {
                    cout << "Palabra no aceptada, no existe transicion para (" << curr_state << ", " << word[curr_char] << ")" << endl;
                    accepted=false;
                    break;
                }

                curr_state = it->second.next_state;
                word[curr_char] = it->second.write;
                if (it->second.direction == 'I') {
                    if (curr_char == 0) {
                        word = "B" + word;
                    } else {
                        curr_char--;
                    }
                }
                else if (it->second.direction == 'D') {
                    if (curr_char+1 < word.size()) curr_char++;
                    else {
                        word+="B";
                        curr_char++;
                    }
                }
            }

            if (accepted == true) cout << "Palabra aceptada." << endl;
            cout << "Ingrese palabra (n=terminar): ";
        }    
        string choice;
        cout << "Desea repetir?: (y | n) "; getline(cin, choice);
        if (choice == "n") repeat=false;
    }


    return 0;
}
