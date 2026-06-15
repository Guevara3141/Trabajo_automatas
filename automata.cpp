// g++ `wx-config --cxxflags` -o automata automata.cpp `wx-config --libs`; ./automata
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <string>
#include <map>
#include <sstream>
#include <iostream>

using namespace std;

struct TransitionResult {
    string next_state;
    char write;
    char direction;
};

void printTape(string& word, int& word_pos) {
    if (word_pos < 0) {
        word_pos = 0;
    }
    else if (word_pos >= word.size()) {
        word_pos = word.size();
        word = word + "B";
    }

    cout << "ultimo estado de la cinta:\n";
    for (int i = 0; i < word.size(); ++i) {
        if (i < word.size() - 1) cout << word[i] << ", ";
        else cout << word[i] << "\n";
    }

    for (int i = 0; i < word_pos * 3; ++i) cout << " ";cout << "^\n";
}

class MainFrame : public wxFrame {
public: 
    MainFrame();
private:
    wxTextCtrl *logOutput; 
    wxTextCtrl *wordInput;
    wxListView *m_transitionsView; // para que tenga acceso a las transiciones

    string m_word;
    string m_initialState;
    string m_finalState;
    map<pair<string, char>, TransitionResult> m_table;
 
    void OnSimulateWord();
    void OnEstablishInitialStateClick(wxCommandEvent& event);
    void OnEstablishFinalStateClick(wxCommandEvent& event);
    void OnDeleteSelectedClick(wxCommandEvent& event);
    void OnClearAllClick(wxCommandEvent& event);
    void ReadTransitionAndSave(const string& a, const string& b);

    void setWord(string word) { m_word = word; }
    void setInitialState(string state) { m_initialState = state; }
    void setFinalState(string state) { m_finalState = state; }
};

void MainFrame::OnEstablishInitialStateClick(wxCommandEvent& event) {
    wxString userInput = wxGetTextFromUser("Ingrese estado inicial:", "estado inicial", m_initialState, this);
    
    if (!userInput.IsEmpty()) {
        setInitialState(string(userInput));
    }
}

void MainFrame::OnEstablishFinalStateClick(wxCommandEvent& event) {
    wxString userInput = wxGetTextFromUser("Ingrese estado final:", "estado final", m_finalState, this);
    
    if (!userInput.IsEmpty()) {
        setFinalState(string(userInput));
    }
}

// borra un elemento seleccionado por el cursor de la tabla visual y de m_table
void MainFrame::OnDeleteSelectedClick(wxCommandEvent& event) {
    long selectedIndex = m_transitionsView->GetFirstSelected();
    if (selectedIndex == -1) {
        wxMessageBox("seleccione una transicion de la tabla.", "Aviso", wxOK | wxICON_INFORMATION);
        return;
    }

    string state = m_transitionsView->GetItemText(selectedIndex, 0).ToStdString();
    string simbolStr = m_transitionsView->GetItemText(selectedIndex, 1).ToStdString();
    char simbol = (!simbolStr.empty()) ? simbolStr[0] : ' ';

    auto key = make_pair(state, simbol);
    m_table.erase(key);

    m_transitionsView->DeleteItem(selectedIndex);
}

// vacia m_table y la tabla visual
void MainFrame::OnClearAllClick(wxCommandEvent& event) {
    m_table.clear();
    m_transitionsView->DeleteAllItems();
    
    wxStreamToTextRedirector redirect(logOutput);
    cout << "Automata eliminado por completo.\n";
}

// intenta una palabra simulando un autómata con cinta semi infinita
void MainFrame::OnSimulateWord() {
    string word = m_word;
    const string& initial_state = m_initialState;
    const string& final_state = m_finalState;
    const map<pair<string, char>, TransitionResult>& table = m_table; 

    // redirecciona cout para que imprima en el log por esta cláusula
    wxStreamToTextRedirector redirect(logOutput);

    cout << "intentado palabra: " << word << "\n";
    int word_pos = 0;
    string curr_state = initial_state;

    int pasos = 0;
    int MAX_PASOS = 1000;

    while (pasos++ < MAX_PASOS) {
        if (word_pos < 0) {
            // falla aquí porque no se puede ir a la izquierda
        }
        else if (word_pos >= word.size()) {
            word = word + "B";
        }

        char c = word[word_pos];

        auto table_input = make_pair(curr_state, c);

        // Si no hay transición se rechaza
        if (table.find(table_input) == table.end()) {
            cout << "Palabra rechazada, transicion (" << curr_state << ", " << c << ") no existe.\n" ;
            printTape(word, word_pos);
            return;
        }

        TransitionResult transition = table.at(table_input);

        // intenta detectar un bucle infinito si se mueve a la derecha ya estando en B y no cambia el estado 
        if (c == 'B' && transition.direction == 'D' && transition.next_state == curr_state) {
            cout << "Palabra rechazada.\n";
            printTape(word, word_pos);
            return;
        }

        cout << "(" << curr_state << ", " << c << ")     word = " << word << "\n";

        // Si la transición nos lleva al estado final aceptamos
        if (transition.next_state == final_state) {
            cout << "Palabra aceptada.\n";
            return;
        }

        if (word_pos == 0 && transition.direction == 'I' ) {
            cout << "Transicion (" << curr_state << ", " << c << ") no es valida para una cinta semi-infinita.\n";
        }
        
        cout << "-> ";
        
        word[word_pos] = transition.write;
        
        curr_state = transition.next_state;
        if (transition.direction == 'D') {
            ++word_pos;
        }
        else if (transition.direction == 'I') {
            --word_pos;
        }
        else {
            cout << "Error en la direccion\n";
            return;
        }
    }

    cout << "palabra rechazada: Limite de pasos excedido.\n";
    printTape(word, word_pos);
    return;

}

void MainFrame::ReadTransitionAndSave(const string& a, const string& b) {
    pair<string, char> primero;
    TransitionResult segundo;

    string temp;
    
    stringstream ss_key(a);
    if (!getline(ss_key, primero.first, ',') || primero.first.empty()) {
        wxMessageBox("Falta el estado origen.", "Error de Formato", wxOK | wxICON_ERROR);
        return;
    }
    if (!getline(ss_key, temp) || temp.empty()) {
        wxMessageBox("Falta el simbolo o la coma de separacion.", "Error de Formato", wxOK | wxICON_ERROR);
        return;
    }
    primero.second = temp[0];
    
    stringstream ss_val(b);
    if (!getline(ss_val, segundo.next_state, ',') || segundo.next_state.empty()) {
        wxMessageBox("Falta el siguiente estado.", "Error de Formato", wxOK | wxICON_ERROR);
        return;
    }
    if (!getline(ss_val, temp, ',') || temp.empty()) {
        wxMessageBox("Falta el simbolo de escritura.", "Error de Formato", wxOK | wxICON_ERROR);
        return;
    } 
    segundo.write = temp[0];
    if (!getline(ss_val, temp) || temp.empty()) {
        wxMessageBox("Falta la direccion.", "Error de Formato", wxOK | wxICON_ERROR);
        return;
    } 
    segundo.direction = temp[0];

    // Validar estrictamente que la dirección sea válida ('I' o 'D')
    if (segundo.direction != 'I' && segundo.direction != 'D') {
        wxMessageBox("La direccion debe ser 'I' o 'D'.", "Direccion Invalida", wxOK | wxICON_ERROR);
        return;
    }
    
    m_table[primero] = segundo;

    long itemIndex = m_transitionsView->GetItemCount();
    m_transitionsView->InsertItem(itemIndex, primero.first);
    m_transitionsView->SetItem(itemIndex, 1, string(1, primero.second));
    m_transitionsView->SetItem(itemIndex, 2, segundo.next_state);
    m_transitionsView->SetItem(itemIndex, 3, string(1, segundo.write));
    m_transitionsView->SetItem(itemIndex, 4, string(1, segundo.direction));
}

class MainApp : public wxApp {
public:
    virtual bool OnInit() {
        MainFrame* frame = new MainFrame();
        frame->Show(true);
        return true;
    }
};
wxIMPLEMENT_APP(MainApp);

MainFrame::MainFrame() : wxFrame(nullptr, wxID_ANY, "Maquinas de Turing", wxDefaultPosition, wxSize(850, 650)) {
    
    wxPanel* mainPanel = new wxPanel(this, wxID_ANY);

    wxBoxSizer* mainBox = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* leftColumn = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* rightColumn = new wxBoxSizer(wxVERTICAL);

    // LEFT COLUMN
    wxStaticBoxSizer* transitionBox = new wxStaticBoxSizer(wxVERTICAL, mainPanel, "1. Construir Automata (CASE SENSITIVE)");
    transitionBox->Add(new wxStaticText(mainPanel, wxID_ANY, "Transicion [estado],[simbolo]:"), 0, wxALL, 2);
    wxTextCtrl* transitionKey = new wxTextCtrl(mainPanel, wxID_ANY, "");
    transitionBox->Add(transitionKey, 0, wxEXPAND | wxBOTTOM, 5);

    transitionBox->Add(new wxStaticText(mainPanel, wxID_ANY, "Modificacion [siguiente_estado],[simbolo_nuevo],[direccion]:"), 0, wxALL, 2);
    wxTextCtrl* transitionVal = new wxTextCtrl(mainPanel, wxID_ANY, "");
    transitionBox->Add(transitionVal, 0, wxEXPAND | wxBOTTOM, 10);

    wxButton* addTransition = new wxButton(mainPanel, wxID_ANY, "Guardar Transicion");
    transitionBox->Add(addTransition, 0, wxEXPAND | wxBOTTOM, 10);
    
    addTransition->Bind(wxEVT_BUTTON, [this, transitionKey, transitionVal](wxCommandEvent& event) {
        string inputFromTransitionKey = string(transitionKey->GetValue());
        string inputFromTransitionVal = string(transitionVal->GetValue());

        if(!inputFromTransitionKey.empty() && !inputFromTransitionVal.empty()) {
            this->ReadTransitionAndSave(inputFromTransitionKey, inputFromTransitionVal);
            transitionKey->Clear();
            transitionVal->Clear();
        }
    });

    // Estados
    wxStaticBoxSizer* stateAssignBox = new wxStaticBoxSizer(wxVERTICAL, mainPanel, "2. Configurar Estados y Transiciones");
    stateAssignBox->Add(new wxStaticText(mainPanel, wxID_ANY, "Seleccione transicion de la tabla:"), 0, wxBOTTOM, 5);
    wxButton* SetInitial = new wxButton(mainPanel, wxID_ANY, "Establecer Estado Inicial");
    wxButton* SetFinal = new wxButton(mainPanel, wxID_ANY, "Establecer Estado Final");
    wxButton* DeleteSelected = new wxButton(mainPanel, wxID_ANY, "Eliminar Estado");
    wxButton* ClearAll = new wxButton(mainPanel, wxID_ANY, "Eliminar Automata");

    stateAssignBox->Add(SetInitial, 0, wxEXPAND | wxBOTTOM, 5);
    stateAssignBox->Add(SetFinal, 0, wxEXPAND | wxBOTTOM, 5);
    stateAssignBox->Add(DeleteSelected, 0, wxEXPAND | wxBOTTOM, 5);
    stateAssignBox->Add(ClearAll, 0, wxEXPAND | wxBOTTOM, 10);

    SetInitial->Bind(wxEVT_BUTTON, &MainFrame::OnEstablishInitialStateClick, this); 
    SetFinal->Bind(wxEVT_BUTTON, &MainFrame::OnEstablishFinalStateClick, this); 
    DeleteSelected->Bind(wxEVT_BUTTON, &MainFrame::OnDeleteSelectedClick, this);
    ClearAll->Bind(wxEVT_BUTTON, &MainFrame::OnClearAllClick, this);

    // Procesamiento
    wxStaticBoxSizer* wordBox = new wxStaticBoxSizer(wxVERTICAL, mainPanel, "3. Ingresar Palabra");
    wordInput = new wxTextCtrl(mainPanel, wxID_ANY, "");
    wxButton* simulateBtn = new wxButton(mainPanel, wxID_ANY, "Procesar Palabra");
    
    wordBox->Add(wordInput, 0, wxEXPAND | wxBOTTOM, 5);
    wordBox->Add(simulateBtn, 0, wxEXPAND);

    simulateBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) {
        m_word = string(wordInput->GetValue());

        if (m_word != "" && m_initialState != "" && m_finalState != "") {
            this->OnSimulateWord();
        } else {
            wxStreamToTextRedirector redirect(logOutput);
            cout << "Asegurese de tener Palabra, Estado Inicial y Estado Final validos.\n";
        }
    });
    
    leftColumn->Add(transitionBox, 0, wxEXPAND | wxALL, 10);
    leftColumn->Add(stateAssignBox, 0, wxEXPAND | wxALL, 10);
    leftColumn->Add(wordBox, 0, wxEXPAND | wxALL, 10);

    // RIGHT COLUMN
    m_transitionsView = new wxListView(mainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    m_transitionsView->AppendColumn("Estado Origen", wxLIST_FORMAT_LEFT, 100);
    m_transitionsView->AppendColumn("Letra", wxLIST_FORMAT_LEFT, 50);
    m_transitionsView->AppendColumn("Siguiente Estado", wxLIST_FORMAT_LEFT, 120);
    m_transitionsView->AppendColumn("Escritura", wxLIST_FORMAT_LEFT, 80);
    m_transitionsView->AppendColumn("Direccion", wxLIST_FORMAT_CENTER, 70);
    
    logOutput = new wxTextCtrl(mainPanel, wxID_ANY, "Terminal logs:\n", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    wxFont monoFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    logOutput->SetFont(monoFont);rightColumn->Add(m_transitionsView, 4, wxEXPAND | wxTOP | wxRIGHT | wxBOTTOM, 15);
    
    rightColumn->Add(logOutput, 2, wxEXPAND | wxRIGHT | wxBOTTOM, 15);mainBox->Add(leftColumn, 2, wxEXPAND);
    
    mainBox->Add(rightColumn, 5, wxEXPAND);
    mainPanel->SetSizer(mainBox);
}
