from time import time
import os, sys
from antichain_link import *
from math import sqrt

class NotFileSpecified(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

class SyntaxErrorInputFile(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

if len(sys.argv) >= 2:
    print_atc = False
    if len(sys.argv) >= 3:
        if sys.argv[2] == "print_atc" :
            print_atc = True
    file_name = sys.argv[1]
    file = open(file_name, 'r')
    line = file.readline()

    # reading comments (this lines are skipped)
    split_line =  line.rsplit()
    while split_line[0] == "c" :
        #print "comment : " + repr(line)
        line = file.readline()
        split_line =  line.rsplit()

    #"p cnf" line
    if (len(split_line) == 4) and (split_line[0] == "p") \
            and (split_line[1] == "cnf")  :
        nbr_max_lit = int(split_line[2]) * 2
        nbr_clause = int(split_line[3]) 
        lines_to_read = file.readlines()
        words_to_read = (lines_to_read.pop(0)).rsplit()
        el = words_to_read.pop(0)
        #initialization
        if el == "a" :
            exists_is_next = True
        elif el == "e" :
            exists_is_next = False
        else :
            raise SyntaxErrorInputFile("No \"exists\" and \"for all\"")
        el = words_to_read.pop(0)

        #forall/exists lines
        ae = []
        tmp_ae = []
        while lines_to_read :
            if el == "0" :
                ae.append(tmp_ae)
                tmp_ae = []
                if not words_to_read :
                    words_to_read = (lines_to_read.pop(0)).rsplit()
                el = words_to_read.pop(0)
                if el == "a" :
                    #print "a"
                    if exists_is_next :
                        raise SyntaxErrorInputFile("No alterning e and a")
                    exists_is_next = not exists_is_next
                elif el == "e" :
                    #print "e"
                    if not exists_is_next :
                        raise SyntaxErrorInputFile("No alterning e and a")
                    exists_is_next = not exists_is_next
                else :
                    break
            else :
                tmp_ae.append(int(el))
            if not words_to_read :
                words_to_read = (lines_to_read.pop(0)).rsplit()
            el = words_to_read.pop(0)
        
        #atc_manager creation
        atc_mng = InitAntichainManager(c_uint(nbr_max_lit), c_uint(nbr_clause))
        id_clause = 0
        #print "begin clause (" + str(id_clause) + ")"
        
        # description of the clauses (atc_manager clauses)
        cont = True # if true the loop continue, else stop
        while cont :
            if el == "0" :
                #print "begin clause (" + str(id_clause) + ")"
                id_clause = id_clause + 1
            else :
                InitLitClause(atc_mng, c_uint(id_clause), \
                                   c_uint(get_id_lit((int(el)))))
                #print "add elem (" + el + ")"
            if not words_to_read :
                if lines_to_read :
                    words_to_read = (lines_to_read.pop(0)).rsplit()
                else :
                    cont = False
            if cont :
                el = words_to_read.pop(0)

        if id_clause != nbr_clause :
            raise SyntaxErrorInputFile("Number of clauses is not correct!")
        

        #starting atc creation
        atc_begin = InitAntichain(atc_mng) #mod
        #AntichainManagerPrint(atc_mng)
        #print ae
        #print ""
        start_time = time()
        stop = False
        while ae and not stop :
            int_ae = ae.pop() #aussi une suite qui est que des forall ou 
            #exists consecutifs
            while int_ae and not stop :
                if print_atc :
                    AntichainPrint(atc_begin)
                if IsTrivial(atc_begin) :
                    stop = True
                if exists_is_next : # c est a dire on est dans for all
                    tmp_iae = int_ae.pop()
                    if print_atc :
                        print ''
                        print 'ForAll ', tmp_iae
                    atc_begin = PreForAll(atc_begin,get_id_lit_ae(tmp_iae))
                    if print_atc :
                        print 'fin : ', (time() - start_time)
                        #print 'ATC Length', atc_length
                        print 'atc sortie: '
                else :
                    tmp_iae = int_ae.pop();
                    if print_atc : 
                        print ''
                        print 'Exists ', tmp_iae
                    atc_begin = PreExists(atc_begin,get_id_lit_ae(tmp_iae))
                    if print_atc :
                        print 'fin : ', (time() - start_time)
                        #print 'ATC Length', atc_length
                        print 'atc sortie: '
            #print ""
            exists_is_next = not exists_is_next
        print "ANTICHAINE FINALE - sec:", (time() - start_time)
        AntichainPrint(atc_begin)
        #AntichainManagerPrint(atc_mng)
    else :
        raise SyntaxErrorInputFile("No \"p\" line")
else :
    raise NotFileSpecified("Please specify the input file")
