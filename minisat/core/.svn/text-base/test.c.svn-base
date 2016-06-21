// g++ -I ../mtl/ Solver.o  test.c -o t.exe

#include "Solver.h"

int main(int argc, char** argv)
{
    Solver      S;
    Solver      T;
    vec<Lit> lits;
    vec<Lit> lits2;
    vec<Lit> lits3;
    vec<Lit> lits4;
    vec<Lit> lits5;
    vec<Lit> lits6;
    vec<Lit> lits0;
    lits.clear();
    lits2.clear();
    T.newVar();
    T.newVar();
    T.newVar();
    T.newVar();
    T.newVar();
    T.newVar();
    T.newVar();
    T.newVar();
    S.newVar();
    S.newVar();
    S.newVar();
    S.newVar();
    S.newVar();
    S.newVar();
    S.newVar();
    S.newVar();
    lits0.clear();
    lits0.push(Lit(0));
    unsigned int i;
    int parsed_lit = -1;
    lits.push( (parsed_lit > 0) 
               ? Lit(abs(parsed_lit)-1) : ~Lit(abs(parsed_lit)-1) );
    parsed_lit = 1;
    lits.push( (parsed_lit > 0) 
               ? Lit(abs(parsed_lit)-1) : ~Lit(abs(parsed_lit)-1) );
    S.addClause(lits);
    T.addClause(lits);
    parsed_lit = 2;
    printf("lalla1\n");
    lits3.push( (parsed_lit > 0) 
               ? Lit(abs(parsed_lit)-1) : ~Lit(abs(parsed_lit)-1) );
    parsed_lit = 3;
    lits3.push( (parsed_lit > 0) 
               ? Lit(abs(parsed_lit)-1) : ~Lit(abs(parsed_lit)-1) );
    parsed_lit = -4;
    lits3.push( (parsed_lit > 0) 
               ? Lit(abs(parsed_lit)-1) : ~Lit(abs(parsed_lit)-1) );
    parsed_lit = -2;
    lits4.push( (parsed_lit > 0) 
               ? Lit(abs(parsed_lit)-1) : ~Lit(abs(parsed_lit)-1) );
    parsed_lit = 5;
    lits4.push( (parsed_lit > 0) 
               ? Lit(abs(parsed_lit)-1) : ~Lit(abs(parsed_lit)-1) );
    parsed_lit = -6;
    lits4.push( (parsed_lit > 0) 
               ? Lit(abs(parsed_lit)-1) : ~Lit(abs(parsed_lit)-1) );
    parsed_lit = 3;
    lits5.push( (parsed_lit > 0) 
               ? Lit(abs(parsed_lit)-1) : ~Lit(abs(parsed_lit)-1) );
    parsed_lit = -5;
    lits5.push( (parsed_lit > 0) 
               ? Lit(abs(parsed_lit)-1) : ~Lit(abs(parsed_lit)-1) );
    parsed_lit = 10;
    lits5.push( (parsed_lit > 0) 
               ? Lit(abs(parsed_lit)-1) : ~Lit(abs(parsed_lit)-1) );
    parsed_lit = -3;
    lits6.push( (parsed_lit > 0) 
               ? Lit(abs(parsed_lit)-1) : ~Lit(abs(parsed_lit)-1) );
    parsed_lit = -7;
    lits6.push( (parsed_lit > 0) 
               ? Lit(abs(parsed_lit)-1) : ~Lit(abs(parsed_lit)-1) );
    parsed_lit = -10;
    lits6.push( (parsed_lit > 0) 
               ? Lit(abs(parsed_lit)-1) : ~Lit(abs(parsed_lit)-1) );
    lits6.push( (parsed_lit > 0) 
               ? Lit(abs(parsed_lit)-1) : ~Lit(abs(parsed_lit)-1) );
    parsed_lit = -100;
    lits2.push( (parsed_lit > 0) ? Lit(abs(parsed_lit)-1) : ~Lit(abs(parsed_lit)-1) );
    parsed_lit = 100;
    lits2.push( (parsed_lit > 0) ? Lit(abs(parsed_lit)-1) : ~Lit(abs(parsed_lit)-1) );
    S.addClause(lits2);
    T.addClause(lits2);
    printf("lalla2\n");
    S.addClause(lits3);
    T.addClause(lits3);
    printf("lalla3\n");
    printf("Snvas %i \n", S.nVars());
    S.addClause(lits4);
    T.addClause(lits4);
    printf("lalla4\n");
    S.addClause(lits5);
    T.addClause(lits5);
    printf("lalla5\n");
    S.addClause(lits6);
    T.addClause(lits6);
    printf("lallala\n");
    bool ret = S.solve();
    printf(ret ? "SATISFIABLE\n" : "UNSATISFIABLE\n");
    ret = T.solve();
    printf(ret ? "SATISFIABLE\n" : "UNSATISFIABLE\n");
}
