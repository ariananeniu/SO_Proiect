# Documentație utilizare AI - Proiect SO Etapa 1

Student: Neniu Ariana-Denisa
Unealtă folosită: Google Gemini

1. Prompt-uri utilizate
- "Generează o funcție C numită parse_condition care să împartă un string de tipul câmp:operator:valoare în componente."
- "Ajută-mă cu o funcție match_condition care să verifice dacă o structură de tip Report îndeplinește condițiile de filtrare."

2. Cod generat
AI-ul a generat structura logică a funcțiilor folosind `sscanf` pentru parsare și o serie de instrucțiuni `if/else` pentru compararea valorilor din structura `Report`.

3. Modificări și justificări
- Corecție Tipuri de Date: Am modificat modul în care este tratat `timestamp` (time_t) pentru a permite comparații numerice corecte.
- Gestionare Permisiuni: Am integrat apeluri `stat()` și `chmod()` în funcțiile de adăugare și logare, deoarece codul generat inițial nu ținea cont de restricțiile specifice de acces (manager vs inspector) cerute în proiect.
- Formatare Output: Am schimbat specificatorul de format în `%lld` pentru dimensiunea fișierului, pentru a evita avertismentele de compilare pe arhitecturi de 64 de biți.

4. Concluzii / Ce am învățat
Am învățat cum se face parsarea robustă a șirurilor de caractere în C și cât de critică este gestionarea manuală a bitilor de permisiune într-un mediu de sistem. De asemenea, am observat că AI-ul poate genera o logică de bază corectă, dar aceasta trebuie întotdeauna adaptată manual la regulile specifice de securitate ale sistemului de operare.