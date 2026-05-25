Student: Neniu Ariana-Denisa
Unealta folosita: Google Gemini

Faza 1
1. Prompt-uri utilizate:
    1. "Genereaza o functie C numita parse_condition care sa imparta un string de tipul camp:operator:valoare in componente."
    2. "Ajuta-ma cu o functie match_condition care sa verifice daca o structura de tip Report indeplineste conditiile de filtrare."
2. Cod generat:
    AI-ul a generat structura logica a functiilor folosind sscanf pentru parsare si o serie de instructiuni if/else pentru compararea valorilor din structura Report.
3. Modificari si justificari
    1. Corectie Tipuri de Date: Am modificat modul in care este tratat timestamp (time_t) pentru a permite comparatii numerice corecte.
    2. Gestionare Permisiuni: Am integrat apeluri stat() si chmod() in functiile de adaugare si logare, deoarece codul generat initial nu tinea cont de restrictiile specifice de acces (manager vs inspector) cerute in proiect.
    3. Formatare Output: Am schimbat specificatorul de format in %lld pentru dimensiunea fisierului, pentru a evita avertismentele de compilare pe arhitecturi de 64 de biti.
4. Concluzii / Ce am invatat
Am invatat cum se face parsarea robusta a sirurilor de caractere in C si cat de critica este gestionarea manuala a bitilor de permisiune intr-un mediu de sistem. De asemenea, am observat ca AI-ul poate genera o logica de baza corecta, dar aceasta trebuie intotdeauna adaptata manual la regulile specifice de securitate ale sistemului de operare.

Faza 2: Procese si Semnale
Pentru faza a doua, am scris logica programului conform cursului, dar am folosit AI-ul pentru a rezolva erori de sintaxa si a clarifica apelurile de sistem.
1. Ce prompt am dat: 
    1. "Cum dau corect argumentele functiei execlp ca sa rulez comanda rm -rf pe un anumit director dintr-un proces copil?"
    2. "De ce imi apare mesajul '[AVERTISMENT] Legatura intrerupta detectata' după ce șterg un district cu remove_district?"
2. Ce a generat: 
    1. Mi-a explicat ca sintaxa corecta este execlp("rm", "rm", "-rf", nume_director, NULL);.
    2. Mi-a explicat ca trebuie sa conditionez apelul cu if (strcmp(command, "remove_district") != 0)
3. Ce am schimbat/invatat: Implementasem deja fork(), dar nu stiam exact cum sa pasez argumentele. Am invatat ca la familia exec, primul argument este executabilul, al doilea este numele cu care ruleaza (argv[0]), iar la final trebuie pus obligatoriu NULL. "handle_symlink" era apelat pentru orice comandă în main, inclusiv imediat după remove_district, când districtul era deja șters. Astfel se crea un symlink nou către un reports.dat inexistent. 

Faza 3: Arhitectura Multi-Proces si Comunicare (IPC)
1. Ce prompt am dat: 
    1. "De ce imi apare eroarea 'Bad file descriptor' de 7 ori la rand si hub-ul nu functioneaza cand apelez execlp ca sa calculez scorurile pentru 3 districte?"
    2. "Cum pot porni mai multe procese scorer, unul cate unul pentru fiecare district, si cum pot colecta separat output-ul fiecaruia prin pipe-uri diferite?"

2. Ce a generat: 
    1. AI-ul mi-a explicat ca, daca execlp() esueaza, procesul copil nu se opreste automat, ci continua sa execute instructiunile de dupa apelul execlp(). Din acest motiv, este important sa existe perror() si exit(1) imediat dupa execlp(), pentru ca procesul copil sa nu continue din greseala logica programului parinte.
    2. Mi-a explicat ca pentru fiecare district trebuie creat un pipe separat si un proces copil separat, iar parintele trebuie sa pastreze capetele de citire pentru fiecare pipe ca sa adune rezultatele dupa ce procesele scorer ruleaza.


3. Ce am schimbat/invatat: Implementasem corect pipe-urile, fork() si dup2(), dar nu protejasem codul pentru esecul functiei exec. Am invatat cum sa gestionez mai multe pipe-uri si mai multe procese copil in acelasi timp, pentru a construi raportul final combinat.