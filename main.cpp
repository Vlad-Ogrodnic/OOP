#include <iostream>
#include<vector>
#include<algorithm>
#include<limits> //folosit in functia de afiseazaobiecte
#include <unordered_set>//pentru concateneazaSiActualizeaza
#include<exception>//pentru cerinta cu exceptii din tema 2
#include<list>//folosit in functia care afiseaza istoricul de cast-uri facute
using namespace std;
class Calculator;//forward declaration ca mi-a trebuit la nu mai stiu ce



template<typename Repo>//template pentru afisat toate optiectele (suboptiunea 4 din case 2)
void displayRepo(const char* title,
                 vector<Repo*>& vec) {
    cout << title << ":\n";
    for (auto* obj : vec) {
        cout << *obj << "\n";
    }
}
class Registru {
protected:
    static vector<Registru *> registrii;
    static unordered_set<Registru *> dynamicAllocs;
    //ca sa vad care sunt alea alocate dinamic ca sa nu am heap corruption cand fac functia concateneazaSiAcutalizeaza
    static int numarRegistri;
    string nume;
    int memorie;
    string scop;

public:
    Registru(string nume = "eax", int memorie = 32, string scop = "Adunari/Scaderi"); //constructor default
    Registru(const Registru &reg); //constructor de copiere
    virtual  ~Registru(); //destructor
    Registru &operator=(const Registru &reg); //operator= de copiere

    // gettere
    string getNume() const { return nume; }
    int getMemorie() const { return memorie; }
    string getScop() const { return scop; }

    // settere
    void setNume(const string &numeNou) { nume = numeNou; }
    void setMemorie(int memorieNou) { memorie = memorieNou; }
    void setScop(const string &scopNou) { scop = scopNou; }


    static int getNumarRegistri() { return numarRegistri; }
    static vector<Registru *> &getRegistrii() { return registrii; }
    static void adaugaRegistru(Registru *reg) { registrii.push_back(reg); }

    Registru operator+(const Registru &other); // operator+ ca funcție membra


    static Registru *concateneazaSiActualizeaza(Registru *reg1, Registru *reg2);

    //de aici pana la sfarsitul functiei isDynamic sunt folosite pentru a stoca cine a fost alocat dinamic si cine nu, ca sa nu am heap corruption in functia aia de la 3
    static void *operator new(size_t sz) {
        //void* returneaza pointer de orice tip, nu inseamna ca nu returneaza nimic. p e mereu de tip Registru* dar e mai safe cu static_cast
        void *p = ::operator new(sz);
        dynamicAllocs.insert(static_cast<Registru *>(p));
        return p;
    }

    static void operator delete(void *p) noexcept {
        //functie statica=poate fi apelata fara sa fie un obiect creat
        dynamicAllocs.erase(static_cast<Registru *>(p));
        ::operator delete(p);
    }

    static bool isDynamic(const Registru *p) {
        //const pentru ca functia doar verifica
        return dynamicAllocs.find(const_cast<Registru *>(p)) //find nu merge daca e const
               != dynamicAllocs.end();
        // daca find de mai sus nu gaseste atunci o sa fie egale si atunci expresia returneaza 0 adica fals
    }
    virtual void print(ostream &os) const {
        os << "Nume: " << nume
           << ", Memorie: " << memorie
           << ", Scop: "   << scop;
    }
};
int Registru::numarRegistri = 0;
vector<Registru *> Registru::registrii;
unordered_set<Registru *> Registru::dynamicAllocs;


string operator*(const string &string);

ostream &operator<<(ostream &os, const Registru &reg) {
    reg.print(os);
    return os;
}

Registru::Registru(string nume, int memorie, string scop) {
    //constructor de initializare
    this->nume = nume;
    this->memorie = memorie;
    this->scop = scop;
    numarRegistri++;
    adaugaRegistru(this); // Adăugăm registrul în vectorul static
}

Registru::Registru(const Registru &reg) {
    //constructor de copiere
    this->nume = reg.nume;
    this->memorie = reg.memorie;
    this->scop = reg.scop;
    numarRegistri++;
    adaugaRegistru(this); // Adăugăm registrul nou creat în vector
}

Registru &Registru::operator=(const Registru &reg) {
    //operator = de copiere
    if (this != &reg) {
        this->nume = reg.nume;
        this->memorie = reg.memorie;
        this->scop = reg.scop;
    }
    return *this;
}

Registru::~Registru() {
    numarRegistri--;
    for (size_t i = 0; i < registrii.size(); ++i) {
        //sterge registrul din vector
        if (registrii[i] == this) {
            registrii.erase(registrii.begin() + i);
            break;
        }
    }
}


Registru Registru::operator+(const Registru &other) {
    //"alt operator supraîncărcat ca funcție membră"
    return Registru(nume + "_" + other.nume, memorie + other.memorie, "Inmultiri/Impartiri");
}

bool operator==(const Registru &reg1, const Registru &reg2) {
    //"alt operator supraîncărcat ca funcție non-membră"
    return (reg1.getNume() == reg2.getNume()) && (reg1.getMemorie() == reg2.getMemorie()) && (
               reg1.getScop() == reg2.getScop());
}

class RegistruARM : public Registru {
    bool isPrivileged;  // doar ARM are registri accesibili doar în mod privilegiat
    public:
    RegistruARM(string nume="r0", int memorie=32, string scop="general-purpose", bool isPrivileged=1):Registru(nume,memorie,scop), isPrivileged(isPrivileged)
    {}
    RegistruARM(const RegistruARM &other) :Registru(other), isPrivileged(other.isPrivileged)
    {}
    RegistruARM &operator=(const RegistruARM &other) {
        if (this != &other) {
            Registru::operator=(other);
            this->isPrivileged = other.isPrivileged;
        }
        return *this;
    }
    void setPrivileged(bool p){ isPrivileged = p;}
    void print(ostream &os) const override {
        Registru::print(os);

        os << ", Privileged: " << (isPrivileged ? "Da" : "Nu");
    }
};

istream &operator>>(istream &is, Registru &reg) {//e pusa dupa registruARM pentru ca altfel nu ma lasa si nu merge nici cu forward declaration
    string nume, scop;
    int memorie;

    cout << "Introdu numele: ";
    is >> nume;
    cout << "Introdu memoria: ";
    is >> memorie;
    cout << "Introdu scopul: ";
    is.ignore();
    getline(is, scop);
    reg.setNume(nume);
    reg.setMemorie(memorie);
    reg.setScop(scop);
    if (auto arm = dynamic_cast<RegistruARM*>(&reg)) {//dynamic-cast
        string privilegedinput;
        cout << "Este privileged? : ";

        is >> privilegedinput;
        transform(privilegedinput.begin(), privilegedinput.end(),privilegedinput.begin(), ::tolower);
        if (privilegedinput == "da")
            arm->setPrivileged(true);
        else if (privilegedinput == "nu")
            arm->setPrivileged(false);
        else {
            cout<<"Optiune invalida, valoare implicita: Da\n";
            arm->setPrivileged(true);
        }

        is.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    return is;
}







//-------------------------------------------------------------------------------------------------------------------









class Procesor {
protected:
    string nume;
    float viteza;
    int anlansare;
    vector<Registru*> registrii;
    static vector<Procesor *> procesoare; //ca sa le stochez pentru optiunea 2 din meniu unde afisez n obiecte

public:
    Procesor(string nume = "procesor_generic", float viteza = 1.8, int anlansare = 2006);

    Procesor(const Procesor &other);

    Procesor &operator=(const Procesor &other);
    virtual ~Procesor() = default;

    void adaugaRegistru(Registru* reg);

    friend ostream &operator<<(ostream &os, const Procesor &procesor);

    friend istream &operator>>(istream &is, Procesor &p){p.read(is); return is;}
    virtual void read(istream &is);
    virtual void print(ostream &os) const;


    static vector<Procesor *> &getProcesoare() { return procesoare; }
    static void adaugaProcesor(Procesor *p) { procesoare.push_back(p); }
    friend bool contineRegistru(const Procesor &procesor, const string &numeRegistru);
    int calculeazaMemorieRegistri() const;
    void removeRegistru(Registru* reg) {
        registrii.erase(
            std::remove(registrii.begin(), registrii.end(), reg),
            registrii.end()
        );
    }
};

vector<Procesor *> Procesor::procesoare;


// Constructor default
Procesor::Procesor(string nume, float viteza, int anlansare)
    : nume(nume), viteza(viteza), anlansare(anlansare), registrii(1) { adaugaProcesor(this); }

// Constructor de copiere
Procesor::Procesor(const Procesor &other)
    : nume(other.nume), viteza(other.viteza), anlansare(other.anlansare), registrii(other.registrii) {
}

// Operator= de copiere
Procesor &Procesor::operator=(const Procesor &other) {
    if (this != &other) {
        nume = other.nume;
        viteza = other.viteza;
        anlansare = other.anlansare;
        registrii = other.registrii; // Copiaza registrii
    }
    return *this;
}
void Procesor::read(istream &is) {
    cout << "Introdu numele: ";
    is >> nume;
    cout << "Introdu viteza: ";
    is >> viteza;
    cout << "Introdu anul de lansare: ";
    is >> anlansare;

    cout << "Cati registrii are procesorul? ";
    int nr;
    is >> nr;
    is.ignore();
    if (nr < 1) {
        cout << "Eroare: Procesorul trebuie sa aiba cel putin un registru! Am creat default un registru eax!" << endl;
        return;
    }
    registrii.clear();
    for (int i = 0; i < nr; ++i) {
        Registru *r=new Registru();
        cout << "Registru " << i + 1 << ":\n";
        is >> *r;
        registrii.push_back(r);
    }
}
void Procesor::print(ostream &os) const {
    os << "Nume: " << nume
            << ", Viteza: " << viteza
            << " GHz, An Lansare: " << anlansare << "\n";
    os << "Registrii procesorului:\n";
    for (const auto *reg: registrii) {
        os << *reg << "\n";
    }
}

void Procesor::adaugaRegistru(Registru* reg) {
    registrii.push_back(reg);
}

int Procesor::calculeazaMemorieRegistri() const {
    int suma = 0;
    for (auto* reg: registrii) {
        suma += reg->getMemorie();
    }
    return suma;
}

ostream &operator<<(ostream &os, const Procesor &procesor) {
    procesor.print(os);
    return os;
}

/*istream &operator>>(istream &is, Procesor &p) {
    cout << "Introdu numele: ";
    is >> p.nume;
    cout << "Introdu viteza: ";
    is >> p.viteza;
    cout << "Introdu anul de lansare: ";
    is >> p.anlansare;
    cout << "Cati registrii are procesorul? ";
    int nr;
    is >> nr;
    is.ignore();
    if (nr < 1) {
        cout << "Eroare: Procesorul trebuie sa aiba cel putin un registru! Am creat default un registru eax!" << endl;
        return is;
    }
    p.registrii.clear();
    for (int i = 0; i < nr; ++i) {
        Registru r;
        cout << "Registru " << i + 1 << ":\n";
        is >> r;
        p.registrii.push_back(r);
    }
    return is;
}*/

bool contineRegistru(const Procesor &procesor, const string &numeRegistru) {
    for ( auto* reg: procesor.registrii) {//echivalent cu Registru* reg: procesor.registrii
        if (reg->getNume() == numeRegistru) {
            return true;
        }
    }
    return false;
}

//3 clase derivate dintr-o clasa de baza, am ales ca date membre doar chestii unice pentru acel tip de cpu si care variaza in functie de modelul exact
//desi probabil cea mai importanta diferenta e arhitectura, nu am putut sa o adaug pentru ca ar fi fost la fel pentru toate cpus din acea clasa
class Procesor_Intel : public Procesor {
    bool hyperthreading; //si amd are doar ca se cheama altfel
    bool sgx; //software guard extensions
public:
    Procesor_Intel(string nume = "i5-10400F", float viteza = 4.3, int anlansare = 2020, bool hyperthreading = 1,
                   bool sgx = 1) : Procesor(nume, viteza, anlansare), hyperthreading(hyperthreading), sgx(sgx)
    {}

    Procesor_Intel(const Procesor_Intel &other): Procesor(other), hyperthreading(other.hyperthreading), sgx(other.sgx)
    //copy constructor suprascris
    {}

    Procesor_Intel &operator=(const Procesor_Intel &other) {
        if (this != &other) {
            Procesor::operator=(other);
            hyperthreading = other.hyperthreading;
            sgx = other.sgx;
        }
        return *this;
    }

void read(istream &is) override {
        Procesor::read(is);
        string hyperthreadinginput;
        string sgxinput;
        cout << "Are hyperthreading?: ";
        is >> hyperthreadinginput;
        transform(hyperthreadinginput.begin(), hyperthreadinginput.end(),hyperthreadinginput.begin(),::tolower);
        cout<<hyperthreadinginput<<endl;
        if (hyperthreadinginput=="da")
            hyperthreading = true;
        else if (hyperthreadinginput=="nu")
            hyperthreading = false;
        else {
            cout<<"Optiune invalida, valoare implicita: Da\n";
            hyperthreading=true;
        }
        cout << " Suporta SGX: ";
        is>>sgxinput;
        transform(sgxinput.begin(), sgxinput.end(),sgxinput.begin(),::tolower);
        if (sgxinput=="da")
            sgx = true;
        else if (sgxinput=="nu")
            sgx = false;
        else {
            cout<<"Optiune invalida, valoare implicita: Da\n";
            sgx=true;
        }
    }
void print(ostream &os) const override {
    Procesor::print(os);
    os << "hyperthreading: " << (hyperthreading ? "Da " : "Nu ")
            << "sgx: " << (sgx ? "Da " : "Nu ") << endl;
}
    friend istream &operator>>(istream &is, Calculator &calculator);
};

class Procesor_AMD : public Procesor {
    bool pboenabled; //precision boost overdrive
    int ccdCount; //core complex dies
public:
    Procesor_AMD(string nume="Ryzen 5 5600", float viteza=4.4, int anlansare=2022, bool pboenabled=1, int ccdCount=1 ) : Procesor(nume, viteza, anlansare), pboenabled(pboenabled), ccdCount(ccdCount)
    {}
    Procesor_AMD(const Procesor_AMD&other): Procesor(other), pboenabled(pboenabled), ccdCount(other.ccdCount)
    {}
    Procesor_AMD &operator=(const Procesor_AMD &other) {
        if (this != &other) {
            Procesor::operator=(other);
            pboenabled = other.pboenabled;
            ccdCount = other.ccdCount;
        }
        return *this;
    }



    void read(istream &is) override {
        Procesor::read(is);
        string pboinput;
        cout << "Are pbo?: ";
        is >> pboinput;
        transform(pboinput.begin(), pboinput.end(),pboinput.begin(),::tolower);
        cout<<pboinput<<endl;
        if (pboinput=="da")
            pboenabled = true;
        else if (pboinput=="nu")
            pboenabled = false;
        else {
            cout<<"Optiune invalida, valoare implicita: Da\n";
            pboenabled=true;
        }
        cout<< "Cati ccd are?:";
        is>>ccdCount;
    }
    void print(ostream &os) const override {
        Procesor::print(os);
        os<< "pboenabled: " << (pboenabled ? "Da" : "Nu")
        << ", nr_de_ccd: " << ccdCount << endl;
    }
    friend istream &operator>>(istream &is, Calculator &calculator);
};

class Procesor_Apple : public Procesor {
    int gpuCores;
    string device; //pentru ce device e conceput  (A series sau M series dar asta ar fi redundant)
    public:
    Procesor_Apple(string nume="M1",float viteza=3.2 , int anlansare=2020,int gpucores=8 , string device="Desktop"): Procesor(nume,viteza,anlansare), gpuCores(gpucores), device(device)
    {}
    Procesor_Apple(const Procesor_Apple& other): Procesor(other), gpuCores(other.gpuCores), device(other.device)
    {}
    Procesor_Apple &operator=(const Procesor_Apple &other) {
        if (this != &other) {
            Procesor::operator=(other);
            gpuCores = other.gpuCores;
            device = other.device;
        }
        return *this;
    }

    void read(istream &is) override {
        cout << "Introdu numele: ";
        is >> nume;
        cout << "Introdu viteza: ";
        is >> viteza;
        cout << "Introdu anul de lansare: ";
        is >> anlansare;
        cout << "Cati registrii are procesorul? ";
        int nr;
        is >> nr;
        is.ignore();
        if (nr < 1) {//insane attention to detail by me right here REGISTRU R0
            cout << "Eroare: Procesorul trebuie sa aiba cel putin un registru! Am creat default un registru r0!" << endl;
            return;
        }
        registrii.clear();
        for (int i = 0; i < nr; ++i) {
            cout << "Registru " << i + 1 << ":\n";
            Registru* r = new RegistruARM();
            is >> *r;
            registrii.push_back(r);
        }
        cout << "Numar GPU cores: ";
        is >> gpuCores;
        cout << "Device: ";
        is >> device;
        is.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    void print(ostream &os) const override {
        Procesor::print(os);
        os << "nr_cores: " << gpuCores << endl;
        os << "pentru : " << device;
    }
    friend istream &operator>>(istream &is, Calculator &calculator);
};










//optiunile 6 si 7 din meniu

list<string> castHistory;

Procesor* upcastAny(Procesor* derived) {
    if (typeid(*derived) == typeid(Procesor)) {
        return nullptr;
    }
    // altfel e derivat => upcast implicit
    castHistory.push_back(
        "Upcast: " + string(typeid(*derived).name()) + "->Procesor*"
    );
    return derived;
}

Procesor_Intel* downcastToIntel(Procesor* p, list<string>& history) {//pentru exemplu de downcast
    if (auto* intel = dynamic_cast<Procesor_Intel*>(p)) {
        history.push_back("Downcast: Procesor*->Procesor_Intel");
        return intel;
    } else {
        history.push_back("Downcast failed: Procesor* not Procesor_Intel");
        return nullptr;
    }
}
void printCastHistory(const list<string>& history) {
    cout << "Istoric cast-uri:\n";
    for (const auto& msg : history) {
        cout << msg << "\n";
    }
}











template<typename Parent, typename Child>//folosit doar in concateneazaSiActualizeaza dar ar merge si in alte locuri daca ar fi nevoie
void removeChildFromAll(vector<Parent*>& parents,
                        Child* child,
                        void (Parent::*remover)(Child*)) {
    for (auto* p : parents) {
        (p->*remover)(child);
    }
}


Registru *Registru::concateneazaSiActualizeaza(Registru *reg1,
                                               Registru *reg2) {
    auto &lista = getRegistrii();

    reg1->nume += "_" + reg2->nume;
    reg1->memorie += reg2->memorie;
    reg1->scop = "Inmultiri/Impartiri";


    lista.erase(
        remove(lista.begin(), lista.end(), reg2),
        lista.end()
    );

    removeChildFromAll<Procesor,Registru>(Procesor::getProcesoare(),reg2,&Procesor::removeRegistru);
    if (isDynamic(reg2)) {
        delete reg2;
    }

    return reg1;
}











//-------------------------------------------------------------------------------------------------------------------






class Calculator {
    string nume;
    bool prebuilt;
    string sistem_de_operare;
    double pret;
    Procesor *procesor; // Agregare
    static vector<Calculator *> calculatoare;

public:
    // Constructor care primeste un procesor
    Calculator(string nume = "PC", bool prebuilt = false, string sistem_de_operare = "Windows10", double pret = 500.00,
               Procesor *procesor = new Procesor("i5-10400KF", 4.3, 2020))
        : nume(nume), prebuilt(prebuilt), sistem_de_operare(sistem_de_operare), pret(pret), procesor(procesor) {
        if (!procesor) {//asta teoretic nu se executa niciodata
            throw invalid_argument("Calculatorul trebuie sa aiba un procesor!");
        }
        calculatoare.push_back(this);
    }

    friend ostream &operator<<(ostream &os, const Calculator &calculator);

    friend istream &operator>>(istream &is, Calculator &calculator);


    static vector<Calculator *> &getCalculatoare() { return calculatoare; }
};

vector<Calculator *> Calculator::calculatoare;


ostream &operator<<(ostream &os, const Calculator &calculator) {
    os << "Nume: " << calculator.nume
            << ", Prebuilt: " << (calculator.prebuilt ? "Da" : "Nu")
            << ", Sistem de operare: " << calculator.sistem_de_operare
            << ", Pret: " << calculator.pret
            << ", Procesor: " << *calculator.procesor << endl;

    return os;
}

istream &operator>>(istream &is, Calculator &calculator) {// m-am chinuit nu stiu cat la asta
    cout << "Selecteaza tipul de procesor pentru calculator:\n"
         << " 1 - Generic\n"
         << " 2 - Intel\n"
         << " 3 - AMD\n"
         << " 4 - Apple\n"
         << "Optiune (1-4): ";
    int tip;
    is >> tip;
    is.ignore(numeric_limits<streamsize>::max(), '\n');


    delete calculator.procesor;
    switch (tip) {
        case 2:
            calculator.procesor = new Procesor_Intel();
        break;
        case 3:
            calculator.procesor = new Procesor_AMD();
        break;
        case 4:
            calculator.procesor = new Procesor_Apple();
        break;
        case 1:
            default:
                if (tip < 1 || tip > 4)
                    cout << "Optiune invalida, folosesc procesor generic.\n";
        calculator.procesor = new Procesor();
    }
    cout << "Introdu numele calculatorului: ";
    is >> calculator.nume;
    string prebuiltInput;
    cout << "Este prebuilt?: ";
    is >> prebuiltInput;
    transform(prebuiltInput.begin(), prebuiltInput.end(), prebuiltInput.begin(), ::tolower);
    if (prebuiltInput == "da") {
        calculator.prebuilt = true;
    } else if (prebuiltInput == "nu") {
        calculator.prebuilt = false;
    } else {
        cout << "Optiune invalida, valoarea implicita 'Nu'.\n";
        calculator.prebuilt = false;
    }
    cout << "Introdu sistemul de operare (intr-un cuvant): ";
    is >> calculator.sistem_de_operare;
    cout << "Introdu pretul: ";
    is >> calculator.pret;
    cout << "Introdu detaliile procesorului: " << endl;
    is >> *calculator.procesor;

    is.ignore();

    return is;
}


//2 clase proprii pentru exceptii si o clasa abstracta
class RegistruException : public exception {
public:
    virtual const char *what() const noexcept override = 0;

    virtual ~RegistruException() = default;
};

class InsuficientRegistri : public RegistruException {
    string msg;

public:
    InsuficientRegistri(size_t n)
        : msg("Eroare: Trebuie minim 2 registri, exista: " + to_string(n)) {
    }

    const char *what() const noexcept override {
        return msg.c_str();
    }
};

class noProcesor : public RegistruException {
    string msg;

public:
    noProcesor(size_t n)
        : msg("Eroare: Trebuie sa existe un procesor ca sa poti face asta") {
    }

    const char *what() const noexcept override {
        return msg.c_str();
    }
};


void afiseazaObiecte() {
    int tip;
    cout << "\nAlegeti tipul de obiect de afisat:" << endl;
    cout << "1 - Registru" << endl;
    cout << "2 - Procesor" << endl;
    cout << "3 - Calculator" << endl;
    cout<<  "4 - Afiseaza toate obiectele"<<endl;
    cout << "5 - Revenire la meniul principal" << endl;
    cout << "Alege o optiune: ";
    cin >> tip;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    int n;
    switch (tip) {
        case 1: {
            const vector<Registru *> &listaReg = Registru::getRegistrii();
            if (listaReg.empty()) {
                cout << "Nu exista registri de afisat." << endl;
                break;
            }
            cout << "Cati registri doresti sa afisezi? ";
            cin >> n;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (n > listaReg.size())
                n = listaReg.size();
            for (int i = 0; i < n; i++) {
                cout << "Registru " << i + 1 << ": " << *listaReg[i] << endl;
            }
            break;
        }
        case 2: {
            const vector<Procesor *> &listaProc = Procesor::getProcesoare();
            if (listaProc.empty()) {
                cout << "Nu exista procesoare de afisat." << endl;
                break;
            }
            cout << "Cate procesoare doresti sa afisezi? ";
            cin >> n;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (n > listaProc.size())
                n = listaProc.size();
            for (int i = 0; i < n; i++) {
                cout << "Procesor " << i + 1 << ": " << *listaProc[i] << endl;
            }
            break;
        }
        case 3: {
            const vector<Calculator *> &listaCalc = Calculator::getCalculatoare();
            if (listaCalc.empty()) {
                cout << "Nu exista calculatoare de afisat." << endl;
                break;
            }
            cout << "Cate calculatoare doresti sa afisezi? ";
            cin >> n;
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); //ignora restul din buffer pana la newline
            if (n > listaCalc.size())
                n = listaCalc.size();
            for (int i = 0; i < n; i++) {
                cout << "Calculator " << i + 1 << ": " << *listaCalc[i] << endl;
            }
            break;
        }
        case 4: {
            displayRepo<Registru>(
              "Registri",
              Registru::getRegistrii()
            );
            displayRepo<Procesor>(
              "Procesoare",
              Procesor::getProcesoare()
            );
            displayRepo<Calculator>(
              "Calculatoare",
              Calculator::getCalculatoare()
            );
            break;
        }

        case 5:
            return;
        default:
            cout << "Optiune invalida!" << endl;
            break;
    }
}








void uiMeniu() {
    cout << "+++++++++++++++++++++++++++++++" << endl;
    cout << " Introdu 1 pentru a creea un obiect " << endl;
    cout << " Introdu 2 pentru a afisa n obiecte" << endl;
    cout << " Introdu 3 pentru a concatena 2 registri " << endl;
    cout << " Introdu 4 pentru a verifica daca un procesor contine un anumit registru " << endl;
    cout << " Introdu 5 pentru a calcula memoria totala a tuturor registrilor dintr un procesor " << endl;
    cout << " Introdu 6 pentru a face upcast la un procesor derivat"<< endl;
    cout << " Introdu 7 pentru a vedea istoricul de cast-uri facute" <<endl;
    cout << " Pentru a iesi din program apasa 0" << endl;
    cout << "-----------------------------\n" << endl;
}




//singleton
class Meniu {
    static Meniu *instance;
    Meniu()=default;
    public:
    static Meniu *getInstance() {
        if (!instance)
            instance = new Meniu();
        return instance;
    }
    Meniu(const Meniu&) = delete;
    Meniu& operator=(const Meniu&) = delete;

    void meniu() {
    bool stop = false;
    int opt;
    while (stop == false) {
        uiMeniu();
        cout << "Introdu optiunea: ";
        cin >> opt;

        switch (opt) {
            case 1: {
                int subOpt;
                cout << "Ce fel de obiect vrei sa creezi?" << endl;
                cout << "1 - Registru" << endl;
                cout << "2 - Procesor" << endl;
                cout << "3 - Calculator" << endl;
                cout << "4 - Revenire la meniu" << endl;
                cout << "Alege o optiune: ";
                cin >> subOpt;

                switch (subOpt) {
                    case 1: {
                        //bloc try/catch 1
                        try {
                            vector<Procesor *> &listaProcesoare = Procesor::getProcesoare();
                            if (listaProcesoare.empty()) {
                                throw noProcesor(listaProcesoare.size());
                            }
                            cout << "Selecteaza procesorul la care vrei sa asignezi registrul:" << endl;
                            for (size_t i = 0; i < listaProcesoare.size(); i++) {
                                cout << i + 1 << " - " << *listaProcesoare[i] << endl;
                            }

                            int optProc;
                            cout << "Alege un indice (1-" << listaProcesoare.size() << "): ";
                            cin >> optProc;
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');

                            if (optProc < 1 || optProc > static_cast<int>(listaProcesoare.size())) {
                                cout << "Optiune invalida!" << endl;
                                break;
                            }
                            cout << "Alege arhitectura registrului:\n"
                                 << " 1 - x86 (implicit)\n"
                                 << " 2 - ARM\n"
                                 << "Optiune: ";
                            int arch;
                            cin >> arch;
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');

                            Registru* regNou = nullptr;
                            if (arch == 2) {
                                regNou = new RegistruARM();
                            } else {
                                regNou = new Registru();
                            }
                            cin >> *regNou;
                            listaProcesoare[optProc-1]->adaugaRegistru(regNou);
                            break;
                        } catch (noProcesor &ex) {
                            cout << ex.what() << endl;
                            break;
                        }

                    }

                    case 2: {
                        int tip;
                        cout << "Ce tip de procesor vrei sa creezi?\n"
                             << " 1 - Procesor generic\n"
                             << " 2 - Procesor Intel\n"
                             << " 3 - Procesor AMD\n"
                             << " 4 - Procesor Apple\n"
                             << "Alege o optiune: ";
                        cin >> tip;
                        Procesor* procNou = nullptr;
                        switch (tip) {
                            case 1:
                                procNou = new Procesor();
                            break;
                            case 2:
                                procNou = new Procesor_Intel();
                            break;
                            case 3:
                                procNou = new Procesor_AMD();
                            break;
                            case 4:
                                procNou = new Procesor_Apple();
                            break;
                            default:
                                cout << "Opțiune invalida\n";
                        }
                        cin>>*procNou;
                        break;
                    }
                    case 3: {
                        Calculator *calcNou = new Calculator();

                        cin >> *calcNou;
                        break;
                    }
                    case 4:
                        break;
                    default:
                        cout << "Optiune invalida!" << endl;
                }
                break;
            }
            case 2:
                afiseazaObiecte();
                break;
            case 3: {
                const vector<Registru *> &listaReg = Registru::getRegistrii();
                try {
                    //bloc try/catch 2
                    if (listaReg.size() < 2)
                        throw InsuficientRegistri(listaReg.size());

                    cout << "Selecteaza registrii pentru concatenare:" << endl;
                    for (size_t i = 0; i < listaReg.size(); i++) {
                        cout << i + 1 << " - " << *listaReg[i] << endl;
                    }
                    int indice1, indice2;
                    cout << "Alege indicele primului registru (1-" << listaReg.size() << "): ";
                    cin >> indice1;
                    cout << "Alege indicele celui de-al doilea registru (1-" << listaReg.size() << "): ";
                    cin >> indice2;
                    Registru *registruRezultat = Registru::concateneazaSiActualizeaza(
                        listaReg[indice1 - 1], listaReg[indice2 - 1]);
                    cout << "Rezultatul concatenarii este: " << *registruRezultat << endl;
                    break;
                } catch (const InsuficientRegistri &ex) {
                    cout << ex.what() << endl;
                    break;
                }
            }

            case 4: {
                vector<Procesor *> &listaProcesoare = Procesor::getProcesoare();
                if (listaProcesoare.empty()) {
                    cout << "Eroare: Nu exista niciun procesor!" << endl;
                    break;
                }


                cout << "Selecteaza procesorul pe care doresti sa verifici:" << endl;
                for (size_t i = 0; i < listaProcesoare.size(); i++) {
                    cout << i + 1 << " - " << *listaProcesoare[i] << endl;
                }
                int indiceProcesor;
                cout << "Alege un indice (1-" << listaProcesoare.size() << "): ";
                cin >> indiceProcesor;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');


                if (indiceProcesor < 1 || indiceProcesor > static_cast<int>(listaProcesoare.size())) {
                    cout << "Optiune invalida!" << endl;
                    break;
                }
                string numeRegistru;
                cout << "Introdu numele registrului: ";
                cin >> numeRegistru;


                if (contineRegistru(*listaProcesoare[indiceProcesor - 1], numeRegistru)) {
                    cout << "Registrul \"" << numeRegistru << "\" exista in procesorul selectat." << endl;
                } else {
                    cout << "Registrul \"" << numeRegistru << "\" NU exista in procesorul selectat." << endl;
                }
                break;
            }

            case 5: {
                try {
                    //try/catch 3 (bonus)
                    const auto &listaProcesoare = Procesor::getProcesoare();

                    if (listaProcesoare.empty()) {
                        throw noProcesor(listaProcesoare.size());
                    }
                    cout << "Selecteaza un procesor pentru a calcula memoria totala a registrilor:" << endl;
                    for (size_t i = 0; i < listaProcesoare.size(); i++) {
                        cout << i + 1 << " - " << *listaProcesoare[i] << endl;
                    }

                    int indiceProcesor;
                    cout << "Alege un indice (1-" << listaProcesoare.size() << "): ";
                    cin >> indiceProcesor;

                    if (indiceProcesor < 1 || indiceProcesor > static_cast<int>(listaProcesoare.size())) {
                        cout << "Optiune invalida!" << endl;
                        break;
                    }
                    int memorieTotala = listaProcesoare[indiceProcesor - 1]->calculeazaMemorieRegistri();
                    cout << "Memoria totala a registrilor procesorului selectat este: " << memorieTotala << " de biti."
                            << endl;
                    break;
                } catch (noProcesor &ex) {
                    cout << ex.what() << endl;
                    break;
                }
            }
            case 6: {
                auto& vec = Procesor::getProcesoare();
                if (vec.empty()) {
                    cout << "Eroare: nu exista niciun procesor!\n";
                    break;
                }
                cout << "Selecteaza procesorul pentru upcast:\n";
                for (size_t i = 0; i < vec.size(); ++i) {
                    cout << " " << i+1 << " - "
                         << typeid(*vec[i]).name()
                         << "\n";
                }
                cout << "Alege un indice (1-" << vec.size() << "): ";
                int idx;
                cin >> idx;
                if (idx < 1 || idx > (int)vec.size()) {
                    cout << "Optiune invalida!\n";
                    break;
                }
                Procesor* basePtr = upcastAny(vec[idx-1]);
                if (!basePtr) {
                    cout << "Eroare: obiectul selectat nu este derivat deci nu se poate upcasta.\n";
                } else {
                    cout << "Upcast realizat: "
                         << typeid(*vec[idx-1]).name()
                         << " -> Procesor*\n";
                }
                break;
            }
            case 7: {
                if (castHistory.empty()) {
                    cout << "Nu a fost facut niciun upcast/downcast inca\n";
                    break;
                }
                printCastHistory(castHistory);
                break;
            }
            case 0:
                stop = true;
                break;
            default:
                cout << "Optiune nevalida! Incearca din nou.\n";
        }

        system("pause");
        system("cls");
    }
    //sterg tot ca sa nu am memory leak
    for (auto reg: Registru::getRegistrii())
        delete reg;
    Registru::getRegistrii().clear();

    for (auto proc: Procesor::getProcesoare())
        delete proc;
    Procesor::getProcesoare().clear();

    for (auto calc: Calculator::getCalculatoare())
        delete calc;
    Calculator::getCalculatoare().clear();
}
};











Meniu* Meniu::instance = nullptr;

int main() {
    //teste random pe care le-am folosit in trecut ca sa nu stau sa scriu acelasi input de 1000 de ori
    //acum, dupa ce am terminat, nu cred ca mai merg pentru ca trebuie citit neaparat de la tastatura
    /* Registru* eax=new Registru();
     Registru* ebx=new Registru("ebx",32,"calcule");
     Registru* ecx=new Registru(*eax);
     Registru* reg=new Registru("rcx", 64, "calcule");
     Calculator calc1;
     cout<<calc1;*/
     //Procesor* procesor1 = new Procesor("Ryzen55600X", 4.6, 2021);
    // cout << *procesor1 << endl;
    // procesor1->adaugaRegistru(new Registru("eax", 32, "Calcule simple"));
    // cout << *procesor1 << endl;
    /*Procesor_Intel p;
    Procesor_Intel p2("i5-11400k",4.3,2021,1,1);
    Procesor_Intel p3=p;
    cout << p;
    p=p2;
    cout<<p;
    cout<<p3;
    Procesor_AMD AMD;
    cout<<AMD<<endl;
    Procesor_Apple mar;
    cout<<mar;*/
    /*RegistruARM r;
    cout<<r<<endl;*/

    //un exemplu de downcast
    /*Procesor* base = new Procesor_Intel();
    Procesor_Intel* intel = downcastToIntel(base, castHistory);
    delete base;*/
    Meniu::getInstance()->meniu();

    return 0;
}
