/* TODO
une fonction qui fait une union sur un IntSet
rajoute des �l�ments directement....
(en intern)

-----

parcourir les blos pour savoir quelles ont d�j� �t� retir�es de tous
les blocs (faire union de tous les blocs) -> on ne va s'int�resser
qu'� ces clauses l�...

mais alors on a plus la relation lit/clauses o� glissement vers la doite
et pour les blocs aussi...

stocker toutes les modifications � faire pour les appliquer sur
chaque bloc<
*/

/// probl�me avec les just_one plus int�ressant si commence � partir
// de zero
#include <stdio.h>
#include <glib.h>
#include <assert.h>

#ifdef MINISAT
#include "./minisat/core/Solver.h"
#endif


/*****************************************/
/*** GUIntList - list of guint element ***/
/*****************************************/
typedef struct GUIntList
{
  guint data;
  GUIntList* next;
} GUIntList;

void guintlist_print (GUIntList* l)
{
  printf("guintlist_print:\n");
  while(l)
    {
      printf("- %i\n", l->data);
      l = l->next;
    }
  printf("\n");
}

void guintlist_free(GUIntList* list)
{
  GUIntList* l_next;
  GUIntList* l = list;
  while(l)
    {
      l_next = l->next;
      g_free(l);
      l = l_next;
    }
}

GUIntList* guintlist_insert(GUIntList *list,  guint data) 
/* the insertion is sorted, and only one element */
{
  if(list == NULL)
    {
      list = g_new(GUIntList,1);
      list->next = NULL;
      list->data = data;
      return list;
    }
  else
    {
      assert(list->data != data);
      if(list->data > data)
        {
          GUIntList* n_list = g_new(GUIntList,1);
          n_list->next = list;
          n_list->data = data;
          return n_list;
        }
      GUIntList* recu_list = list;
      while(recu_list->next)
        {
          assert(recu_list->next->data != data);
          if(recu_list->next->data > data)
            {
              GUIntList* n_maillon = g_new(GUIntList,1);
              n_maillon->data = data;
              n_maillon->next = recu_list->next;
              recu_list->next = n_maillon;
              return list;
            }
          recu_list = recu_list->next;
        }
      GUIntList* n_maillon = g_new(GUIntList,1);
      n_maillon->data = data;
      n_maillon->next = NULL;
      recu_list->next = n_maillon;
      return list;
    }
}

GUIntList* guintlist_remove(GUIntList *list,  guint data)
{
  if(list == NULL)
    return NULL;
  else
    {
      if(list->data < data)
        {
          GUIntList* recu_list = list;
          while(recu_list->next)
            {
              if(recu_list->next->data == data)
                {
                  GUIntList* tmp = recu_list->next->next;
                  g_free(recu_list->next);
                  recu_list->next = tmp;
                  return list;
                }
              else
                {
                   if(recu_list->next->data > data)
                     return list; // warning? no data to remove?
                }
              recu_list = recu_list->next;
            }
          return list; // warning? no data to remove?
        }
      else
        {
          if(list->data == data)
            {
              GUIntList* ret = list->next;
              g_free(list);
              return ret;
            }
          else
            return list; // warning? no data to remove?
        }
    }
}
  
/*************/
/** guint64 **/
/*************/
guint64 zero = 0;
guint64 one = 1; 
guint64 only_one = ~(guint64)0;

void guint64_print (guint64 x)
{
  gint i = 63;
  while(i >= 0)
    {
      guint64 t = one << i;
      if((t & x) == 0)
        printf("0");
      else
        printf("1");
      i --;
    }
}

gint guint64_compare (guint64 ui1, guint64 ui2) 
{
  if(ui1 == ui2)
    return 0;
  else if (ui1 > ui2)
    return 1;
  else
    return -1;
}

/*******************/
/** IntSetManager **/
/****************** */
typedef struct
{
  guint N; // an IntSet is a subset of { 0, ... N-1 }
  gint64 last_only_one; // for complementation
  guint nbr_guint64; // number of guint64 of the IntSet
  guint64 just_one[64]; // array of 64 bits number precalculate // usefull??
} IntSetManagerStruct;
  
typedef IntSetManagerStruct* IntSetManager;

void IntSetManagerPrint(IntSetManager i_mng)
{
  printf("N : %u\n", i_mng->N);
  printf("loo\n");
  guint64_print(i_mng->last_only_one);
  printf("\n");
  printf("n_g : %i\n", i_mng->nbr_guint64);
  printf("first_jo\n");
  ///guint64_print(i_mng->just_one);
  printf("\n");
}



// An IntSet of N elements is a guint64 tab of (nbr_guint64(N) + 1) elements
guint nbr_guint64 (guint N) 
{
  return ((N-1) / 64); 
}


IntSetManager InitIntSetManager(guint N)
{
  assert(N>0);
  
  IntSetManager ret = g_new(IntSetManagerStruct,1);
  ret->N = N;
  
  guint i;
  for(i = 0; i < 64; i++)
      ret->just_one[i] = one << i;
  
  // cr�ation du last_only_one -> utilis� pour la compl�mentation
    guint n_mod_64 = N % 64;
    if(n_mod_64 == 0)
      {
      ret->last_only_one = only_one;
      }
  else
    {
      guint i;
      guint64 tmp_loo = 0;
      for(i = 0; i < n_mod_64; i++)
        {
          tmp_loo = tmp_loo | (one << i);
        }
      ret->last_only_one = tmp_loo;
    }
  // fin de cette cr�ation

   ret->nbr_guint64 = nbr_guint64(N) + 1;
}


/******************************************************************/
/* INTSET (set encoded with integer - used for clauses and blocs) */
/******************************************************************/
typedef struct
{ 
  IntSetManager mng; // car on utilise des intsets de tailles diff
  guint64* set; 
} IntSetStruct;

typedef IntSetStruct* IntSet;

// release the memory allocated< to the IntSet s
void IntSetRelease(IntSet s)
{
  g_free (s->set);
  g_free (s);
}


void IntSetEmptyInit(IntSetManager mng, IntSet s)
{
  guint ngu = mng->nbr_guint64;
  s->mng = mng;
  s->set = g_new(guint64, ngu);
  guint i;
  for(i = 0; i < ngu; i++)
    s->set[i] = 0;
}

// Create an empty IntSet
IntSet IntSetEmpty(IntSetManager mng)
{
  IntSet new_IntSet = g_new(IntSetStruct,1);
  IntSetEmptyInit(mng,new_IntSet);
  return new_IntSet;
}

//Create an full IntSet
IntSet IntSetFull(IntSetManager mng)
{
  IntSet new_IntSet = g_new(IntSetStruct,1);
  new_IntSet->mng = mng;
  new_IntSet->set = g_new(guint64, mng->nbr_guint64);
  guint i;
  guint ngu_minus1 = mng->nbr_guint64 - 1;
  for(i = 0; i < ngu_minus1; i++)
    new_IntSet->set[i] = only_one;
  new_IntSet->set[ngu_minus1] = mng->last_only_one; // ~mng->last_only_one;
  return new_IntSet;
}

void EmptyOutIntSet (IntSet s)
{
  guint i;
  for(i = 0; i <  s->mng->nbr_guint64; i++)
    s->set[i] = zero;
}


// Print the IntSet s on the screen
void IntSetPrint (IntSet s)
{
  guint ngu = s->mng->nbr_guint64;
  guint i = 0;
  while (i < ngu)
    {
      printf("|");
      guint64_print(s->set[i]);
      printf("|\n");
      i ++;
    }
}

//Compares two IntSets
// return 0 if the intset are equal
// 1 ou -1 if they are diff�rents
gint IntSetCompare (IntSet s1, IntSet s2)
{
  assert(s1->mng == s2->mng);
  guint ngu = s1->mng->nbr_guint64;
  gint ret = 0;
  guint i = 0;
  while(ret == 0 && i < ngu)
    {
      gint compare_level_i = guint64_compare(s1->set[i], s2->set[i]);
      if (compare_level_i == 0)
        i = i++;
      else
        ret = compare_level_i;
    }
  return ret;
}


//Returns TRUE if the two IntSet s1 and s2 are equals; otherwise returns FALSE
gboolean IntSetIsEqual (IntSet s1, IntSet s2)
{
  return (IntSetCompare(s1,s2) == 0);
}


//Returns TRUE if the IntSet s is empty; otherwise returns FALSE
// verifier que le return sort de la boucle
gboolean IntSetIsEmpty (IntSet s)
{
  guint ngu = s->mng->nbr_guint64;
  guint i = 0;
  while(i < ngu)
    {
      gint compare_level_i = guint64_compare(s->set[i], zero);
      if (compare_level_i == 0)
        i = i++;
      else
        return FALSE;
    }
  return TRUE;
}

//Returns the IntSet witch is the results of the union of s1 with s2
IntSet IntSetUnion(IntSet s1, IntSet s2)
{
  assert(s1->mng == s2->mng);
  gint ngI = s1->mng->nbr_guint64;
  IntSet ret = IntSetEmpty(s1->mng);
  guint i;
  for(i = 0; i < ngI; i++)
      ret->set[i] = s1->set[i] | s2->set[i];
  return ret;
}

//Returns the IntSet witch is the results of the exc-union of s1 with s2
IntSet IntSetExcUnion (IntSet s1, IntSet s2)
{
  assert(s1->mng == s2->mng);
  gint ngI = s1->mng->nbr_guint64;
  IntSet ret = IntSetEmpty(s1->mng);
  guint i;
  for(i = 0; i < ngI; i++)
    {
      ret->set[i] = s1->set[i] ^ s2->set[i];
    }
  return ret;
}

//Returns the IntSet witch is the results of the intersection of s1 with s2
IntSet IntSetIntersection(IntSet s1, IntSet s2)
{
  assert(s1->mng == s2->mng);
  gint ngI = s1->mng->nbr_guint64;
  IntSet ret = IntSetEmpty(s1->mng);
  guint i;
  for(i = 0; i < ngI; i++)
      ret->set[i] = s1->set[i] & s2->set[i];
  return ret;
}

void auto_IntSetComplement(IntSet s)
{
  gint ngI_minus1 = s->mng->nbr_guint64 - 1;
  guint i;
  for(i = 0; i < ngI_minus1; i++)
      s->set[i] = s->set[i] ^ only_one;
  s->set[ngI_minus1] = s->set[ngI_minus1] ^ s->mng->last_only_one;
}

//Returns the IntSet witch is the complement of s
IntSet IntSetComplement(IntSet s)
{
  gint ngI_minus1 = s->mng->nbr_guint64 - 1;
  IntSet ret = IntSetEmpty(s->mng);
  guint i;
  for(i = 0; i < ngI_minus1; i++)
      ret->set[i] = s->set[i] ^ only_one;
  ret->set[ngI_minus1] = s->set[ngI_minus1] ^ s->mng->last_only_one;
  return ret;
}

//Returns a copy of the IntSet s
IntSet IntSetCopy(IntSet s)
{
  gint ngI = s->mng->nbr_guint64;
  IntSet ret = IntSetEmpty(s->mng);
  guint i;
  for(i = 0; i < ngI; i++)
    ret->set[i] = s->set[i];
  return ret;
}

// intern function

void intern_IntSetAddElem (IntSet s, long elem)
{
  assert(elem < s->mng->N);
  guint tab_to_add = (guint) (elem / 64);
  guint int_to_add = (guint) elem % 64;
  s->set[tab_to_add] = s->set[tab_to_add] | s->mng->just_one[int_to_add];
}

//Returns an new IntSet witch is the copy of s where the element elem
//is added
IntSet IntSetAddElem (IntSet s, long elem)
{
  IntSet ret = IntSetCopy(s);
  intern_IntSetAddElem(ret,elem);
  return ret;
}


// intern function
void intern_IntSetRmElem (IntSet s, long elem)
{
  assert(elem < s->mng->N);
  guint tab_to_add = (guint) (elem / 64);
  guint int_to_add = (guint) elem % 64;
  s->set[tab_to_add] = (s->set[tab_to_add] | s->mng->just_one[int_to_add]) 
  ^ s->mng->just_one[int_to_add];
}

//Returns an new IntSet witch is the copy of s where the element elem
//is deleted
IntSet IntSetRmElem (IntSet s, long elem)
{
  IntSet ret = IntSetCopy(s);
  intern_IntSetRmElem(ret,elem);
  return ret;
}


// Returns TRUE if the element elem is an element of the IntSet s and
// FALSE in the othercase
gboolean IntSetHasElem (IntSet s, long elem)
{
  guint64* jo = s->mng->just_one;
  guint tab_to_add = (guint) (elem / 64);
  guint N = s->mng->N;
  assert(elem < N);
  guint int_to_add = (guint) elem % 64;
  return (s->set[tab_to_add] & jo[int_to_add])  == 
    jo[int_to_add];
}

// Returns TRUE if the IntSet s1 is a subset of the IntSet s2 and
// FALSE in the othercase
gboolean IntSetIsIncluded (IntSet s1, IntSet s2)
{
  assert(s1->mng == s2->mng);
  guint i = 0;
  gboolean tmp_is_include = TRUE;
  guint64 tmp_union;
  guint ngu = s1->mng->nbr_guint64;
  
  while (i < ngu && tmp_is_include)
    {
      tmp_union = s1->set[i] | s2->set[i];
      tmp_is_include = (tmp_union == s2->set[i]);
      i ++;
    }
  return tmp_is_include;
}

gboolean IntSetIntersecIsEmpty (IntSet s1, IntSet s2)
{
  assert(s1->mng == s2->mng);
  guint i = 0;
  gboolean tmp_intersec_is_empty = TRUE;
  guint64 tmp_intersec;
  guint ngu = s1->mng->nbr_guint64;
  while(i < ngu && tmp_intersec_is_empty)
    {
      tmp_intersec = s1->set[i] & s2->set[i];
      tmp_intersec_is_empty = (zero == tmp_intersec);
      i ++;
    }
  return tmp_intersec_is_empty;
}


/***********************/
/** ANTICHAIN MANAER **/
/***********************/
typedef struct
{
  guint nbr_lit; // the clauses are sub set of {0, .... N-1}
  guint nbr_clauses;
  IntSetStruct* lit_clauses; //matrix giving the relation clauses-lit�raux
       // the matrix is an array of a IntSet; elements of the array are associate
       // to litiraux
  IntSetManager blocs_mng;

  #ifdef OPTIMA
  IntSetStruct* clauses_lit; //matrix giving the relation clauses-lit�raux
       // the matrix is an array of a IntSet; elements of the array are associate
       // to clauses
  IntSetStruct* included_clauses;
  // pas juste � modifier
  // is a array of elements
       // associates to a clauses, the element is an int set representing
       // the set of clauses 0 -> if the clause is not include in it 1 in 
       // the other case
  IntSetStruct* included_clauses_co_welem;
  // idem que included_clause mais compl�mentaire et �l�men dedans...
  GSList* list_iccowe;
  // liste des included_clauses_co_welem
  // included_clauses_co_welem -> complement of included clause with elem
  IntSetManager clauses_mng;
  #endif

  #ifdef MINISAT
  GUIntList** desc_clauses;
  GUIntList* elems_in_bloc; // afin d'�viter des �l�ments qui ne sont plus dans
  // les blocs
  IntSet union_bloc;
  #endif
}  AntichainManagerStruct;

typedef AntichainManagerStruct* AntichainManager;

#ifdef OPTIMA
typedef struct 
{
  guint id_elem;
  guint nbr_included;
} BlockWNbrIncludedStruct;

typedef BlockWNbrIncludedStruct* BlockWNbrI;

gint BlockWNbrICompare (BlockWNbrI b1, BlockWNbrI b2)
{
  if(b1->nbr_included == b2->nbr_included)
      return 0;
  else
    {
      if(b1->nbr_included < b2->nbr_included)
        return 1;
      else
        return -1;
    }
}

gint comp_blockI (gconstpointer a, gconstpointer b)
{
  return BlockWNbrICompare ((BlockWNbrI)a, (BlockWNbrI)b);
}

void CreateIncludedClauses(AntichainManager mng)
/* creates the included_clauses element in function of
the clauses_lit element */
{
  IntSetStruct* included_clauses_co_welem = mng->included_clauses_co_welem;
  
  g_slist_free(mng->list_iccowe);
  mng->list_iccowe = NULL;
  guint i;
  guint j;
  guint nbr_not_included = 0;
 
  for(j = 0; j< mng->nbr_clauses; j++)
    {
      nbr_not_included = 0;
      EmptyOutIntSet(&(mng->included_clauses[j]));
      EmptyOutIntSet(&(mng->included_clauses_co_welem[j]));

      for(i = 0; i < mng->nbr_clauses; i++)
        {
          if(IntSetIsIncluded(&(mng->clauses_lit[j]),&(mng->clauses_lit[i])))
            {
              intern_IntSetAddElem(&(mng->included_clauses[i]),j);
              intern_IntSetAddElem(&(mng->included_clauses_co_welem[j]),i);
              nbr_not_included = nbr_not_included +1;
            }
        }
      auto_IntSetComplement(&(included_clauses_co_welem[j]));
      intern_IntSetAddElem(&(included_clauses_co_welem[j]),j);
      BlockWNbrI blockWNbrI_to_insert = g_new(BlockWNbrIncludedStruct,1);
      blockWNbrI_to_insert->id_elem = j;
      blockWNbrI_to_insert->nbr_included = mng->nbr_clauses - nbr_not_included;
      mng->list_iccowe = g_slist_insert_sorted(mng->list_iccowe,blockWNbrI_to_insert, &comp_blockI);
    }

  //printf("list length %i \n", g_slist_length(mng->list_iccowe));
}
#endif

extern "C" {
// renommer ret_clauses
  AntichainManager InitAntichainManager (guint nbr_lit, guint nbr_clauses)
  {
    AntichainManager ret = g_new(AntichainManagerStruct,1);
    guint i;

    ret->nbr_lit = nbr_lit;
    ret->nbr_clauses = nbr_clauses;

    //ret->clauses_for_minisat = g_new(vec<Lit>, nbr_clauses);

    #ifdef OPTIMA
    ret->list_iccowe = NULL;
    ret->included_clauses = g_new(IntSetStruct, nbr_clauses);
    ret->included_clauses_co_welem = g_new(IntSetStruct, nbr_clauses);
    #endif

    ret->lit_clauses = g_new(IntSetStruct, nbr_lit);
    ret->blocs_mng = InitIntSetManager(nbr_clauses);    
    
    #ifdef OPTIMA
    ret->clauses_lit = g_new(IntSetStruct, nbr_clauses);
    ret->clauses_mng = InitIntSetManager(nbr_lit);
    #endif
    
    for(i = 0; i < nbr_lit; i++)
      IntSetEmptyInit(ret->blocs_mng,&(ret->lit_clauses[i]));

    #ifdef OPTIMA
    for(i = 0; i < nbr_clauses; i++)
      IntSetEmptyInit(ret->clauses_mng,&(ret->clauses_lit[i]));
    #endif

    #ifdef OPTIMA
    for(i = 0; i < nbr_clauses; i++)
      {
        IntSetEmptyInit(ret->blocs_mng,&(ret->included_clauses[i]));
        IntSetEmptyInit(ret->blocs_mng,&(ret->included_clauses_co_welem[i]));
      }
    #endif

    #ifdef MINISAT
    ret->union_bloc = IntSetFull(ret->blocs_mng);
    ret->elems_in_bloc = NULL;

    for(i = 0; i < nbr_clauses; i++)
      {
        ret->elems_in_bloc = guintlist_insert(ret->elems_in_bloc, i);
      }

    ret->desc_clauses = g_new(GUIntList*,nbr_clauses);
    for(i = 0; i < nbr_clauses; i++)
      ret->desc_clauses[i] = NULL;
    #endif

    return ret;
  }
}

extern "C" {
  void InitLitClause(AntichainManager mng, guint clause_id, guint elem)
  {
    intern_IntSetAddElem (&(mng->lit_clauses[elem]), clause_id);
    #ifdef OPTIMA
    intern_IntSetAddElem (&(mng->clauses_lit[clause_id]), elem);
    #endif
    #ifdef MINISAT
    mng->desc_clauses[clause_id] = guintlist_insert
      (mng->desc_clauses[clause_id], elem);
    #endif
      //g_sorted_slist_remove_all()
  }
}

IntSet CellPre (AntichainManager mng, IntSet s, guint elem)
// elem doit �tre entre 0 (pour 1), 1 (pour -1), 2 (pour 2),
// 3 (pour -2)
// Pour tout bit de la Cell qui est �gal � 0, on �value
// la clause identifi� par ce bit, via l'�l�ment
// si la clause a cet �l�ment on met ce bit � 1
// il faut supprimer l'ancienne cell apr�s
{
  IntSet ret = IntSetIntersection(s,&(mng->lit_clauses[elem]));
  return ret;
  
}

typedef struct
{
  GSList* blocs;
  AntichainManager mng;
} AntichainStruct;

typedef AntichainStruct* Antichain;

// supprime la structure de "a" mais pas les IntSet
void AntichainRelease (Antichain a)
{
  g_slist_free (a->blocs);
  g_free(a);
}

extern "C" {
  void AntichainManagerPrint (AntichainManager mng)
  {
  printf("ANTICHAIN MNG PRINT\n");
  printf("nbr_lit : %i\n", mng->nbr_lit);
  printf("nbr_clauses : %i\n", mng->nbr_clauses);
  printf("blocs_mng:\n");
  //IntSetManagerPrint(mng->blocs_mng);
  guint i;
  printf("lit clauses :\n");
  for(i = 0; i < mng->nbr_lit;i++)
    { 
      printf("l %i ->\n", i);
      IntSetPrint(&(mng->lit_clauses[i]));
    }
  #ifdef OPTIMA
  printf("clauses_lit :\n");
  for(i = 0; i < mng->nbr_clauses;i++)
    {
      printf("c %i ->\n", i);
      IntSetPrint(&(mng->clauses_lit[i]));
    }
  printf("included_clauses :\n");
  for(i = 0; i < mng->nbr_clauses;i++)
    {
      printf("c %i ->\n", i);
      IntSetPrint(&(mng->included_clauses[i]));
    }
  #endif
  printf("fin ANTICHAIN MNG PRINT\n");
  printf("\n");
  }
}


extern "C" {
  void AntichainPrint (Antichain a)
  {
  printf("antichain print ------- \n");
  GSList* l = a->blocs;
  while(l)
    {
      IntSetPrint((IntSet)l->data);
      printf("-------------------------------\n");
      l = l->next;
    }
  //AntichainManagerPrint(a->mng);
  printf("fin atc   print ------- \n");
  }
}

extern "C" {
  guint AntichainLength(Antichain a)
  {
    return g_slist_length(a->blocs);
  }
}


Antichain EmptyAntichain (AntichainManager mng)
{
  Antichain ret = g_new(AntichainStruct, 1);
  ret->blocs = NULL;
  ret->mng = mng;
  return ret;
}

extern "C" {
  Antichain InitAntichain (AntichainManager mng)
  {
  IntSet first_cell = IntSetFull(mng->blocs_mng);
  Antichain ret = g_new(AntichainStruct, 1);
  ret->blocs = g_slist_prepend(NULL,first_cell);
  ret->mng = mng;
  
  guint i;
  
  for(i = 0; i < mng->nbr_lit;i++)
    {
      IntSet ret = IntSetComplement(&(mng->lit_clauses[i]));
      mng->lit_clauses[i] = *ret;
      
    }
 
   #ifdef OPTIMA
   CreateIncludedClauses(mng);
   #endif

  return ret;
  }
}

gboolean IsLess (AntichainManager atc_mng, IntSet b1, IntSet b2)
{
  #ifdef OPTIMA
  guint i = 0;
  gboolean tmp_is_less = TRUE;

  GSList* l = atc_mng->list_iccowe;
  
  // ici ne regarder que les �l�ments qui sont dans union de tous les blocs
  // faire une liste simplement chain�e de �a
  while(l)
    {
      BlockWNbrI bI = (BlockWNbrI) l->data;
      if(IntSetHasElem(b1,bI->id_elem))
        {
          //printf("-- %i\n", bI->id_elem);
          
          if(IntSetIntersecIsEmpty(&(atc_mng->included_clauses[bI->id_elem]),b2))
            tmp_is_less = FALSE;
        }
      l = l->next;
    }
  return tmp_is_less;
  #else
  return IntSetIsIncluded(b1,b2);
  #endif
}

// supprime tous le noeud xxx->next si c est inclus � xxx->next->data
void explore_next_and_remove_min(AntichainManager atc_mng, GSList *atc, 
                                 IntSet cell)
{
  while(atc->next)
    {
      if(IsLess(atc_mng, cell,(IntSet) atc->next->data))
        {
          IntSetRelease((IntSet) atc->next->data);
          GSList* poineur_vers_atc = g_slist_delete_link(atc,atc->next);
        }
      else
        {
          atc=atc->next;
          }
    }
  return;
}

GSList* explore_and_replace_min  (AntichainManager atc_mng, GSList *atc, 
                                 IntSet bloc)
{
  if (atc ==  NULL)
    return g_slist_prepend(NULL, bloc);
  else
    {
      GSList* l = atc;
      GSList* before_l = NULL;
      gboolean stop = false;
      IntSet bloc_from_l;
      while(l && !stop)
        {
          bloc_from_l = (IntSet) l->data;
          if (IsLess(atc_mng, bloc_from_l, bloc))
            {
              #ifdef SWAP1
              if(before_l != NULL)
                {
                  IntSet tmp = (IntSet) before_l->data;
                  before_l->data = l->data;
                  l->data = tmp;
                }
              #endif
              #ifdef SWAP2
              if(before_l != NULL)
                {
                  atc = g_slist_prepend(atc, l->data);
                  before_l->next = l->next;
                  g_slist_free_1(l);
                }
              #endif
              IntSetRelease(bloc);
              stop = true;
            }
          else
            {
              if (IsLess(atc_mng, bloc, bloc_from_l))
                {
                  IntSetRelease(bloc_from_l);
                  l->data = bloc;
                  explore_next_and_remove_min(atc_mng, l, bloc);
                  stop = true;
                }
            }
          before_l = l;
          l = l->next;
        }

      if(!stop)
        {
          before_l->next =  g_slist_alloc();
          before_l->next->data = bloc;
        }

      return atc;
    }
}


#ifdef OPTIMA
IntSet RemoveSuperSetClauses (AntichainManager mng, IntSet b)
{
  IntSet handled_elem = IntSetEmpty(b->mng);
  // intset des �l�ments qui ont d�j� �t� regard�
  // si b devient juste cet �l�ment c'est que tout a �t� �limin�...
  GSList* l = mng->list_iccowe;
  IntSet tmp;
  guint i;
  gboolean stop = false;
  
  while(l && !stop)
    {
      BlockWNbrI bI = (BlockWNbrI) l->data;
      if(IntSetHasElem(b,bI->id_elem))
        {
          tmp = IntSetIntersection(b, &(mng->included_clauses_co_welem[bI->id_elem]));
          IntSetRelease(b);
          b = tmp;
          intern_IntSetAddElem (handled_elem, bI->id_elem);
          if(IntSetIsIncluded(b,handled_elem))
            stop = true;
        }
      l = l->next;
    }
  return b;
}
#endif

// suppprime les IntSet qui ne sont pas dans 
// l'antichain
void AntichainAddCell(Antichain a, IntSet c)
{
  #ifdef OPTIMA
  a->blocs = explore_and_replace_min(a->mng, a->blocs,
                                     RemoveSuperSetClauses(a->mng,
                                                           (IntSet)c));
  #else
  a->blocs = explore_and_replace_min(a->mng, a->blocs,c);
  #endif
}

void iter_AddCell(gpointer data, gpointer user_data)
{
  IntSet c = (IntSet) data;
  Antichain atc = (Antichain) user_data;
  AntichainAddCell(atc, c);
 }

guint id_of_lit (gint lit)
{
  if (lit > 0)
    return lit;
  else
    return ((-1) * lit);
}

guint lit_to_var (gint lit)
{
  if (lit > 0)
    return (lit - 1) * 2;
  else
    return (((-1 * lit) - 1) * 2) + 1;
}

#if defined(MINISAT) || defined(OPTIMA)
void UpdateClauseLit(AntichainManager mng, glong var_id_lit)
{
  guint i;
  for(i = 0; i < mng->nbr_clauses; i ++)
    {      
      #ifdef MINISAT
      mng->desc_clauses[i] = guintlist_remove(mng->desc_clauses[i], var_id_lit);
      mng->desc_clauses[i] = guintlist_remove(mng->desc_clauses[i], var_id_lit+1);
      #endif

      #ifdef OPTIMA
      intern_IntSetRmElem (&(mng->clauses_lit[i]), var_id_lit);
      intern_IntSetRmElem (&(mng->clauses_lit[i]), var_id_lit+1);
      #endif
    }

  #ifdef OPTIMA
  CreateIncludedClauses(mng);
  #endif
}
#endif


#ifdef MINISAT
void UpdateElemsInBloc(Antichain a)
{
  AntichainManager mng = a->mng;
  IntSet new_union_bloc = IntSetEmpty(mng->union_bloc->mng);
  IntSet tmp;
  GSList* lb = a->blocs;
  while(lb)
    {
      IntSet b = (IntSet) lb->data;
      tmp = IntSetUnion(new_union_bloc,b);
      IntSetRelease(new_union_bloc);
      new_union_bloc = tmp;
      lb = lb->next;
    }
  
  mng->union_bloc = new_union_bloc;
  
  //elems_in_bloc update
  GUIntList* new_elems_in_bloc = NULL;
  GUIntList* l = mng->elems_in_bloc;

  while(l)
    {  
       if(IntSetHasElem(new_union_bloc,l->data))
         new_elems_in_bloc = guintlist_insert(new_elems_in_bloc,l->data);
       l = l->next;
    }

  guintlist_free(mng->elems_in_bloc);
  mng->elems_in_bloc = new_elems_in_bloc;
}
#endif



Antichain AntichainCrossUnion(Antichain atc1, Antichain atc2)
{
  assert(atc1->mng == atc2->mng);
  Antichain ret = EmptyAntichain(atc1->mng);
  
  GSList* l1 = atc1->blocs;
  GSList* begin_l2 = atc2->blocs;
  GSList* l2;

  while(l1)
    {
      IntSet c1 = (IntSet) l1->data;
      l2 = begin_l2;

      while(l2)
        {
          AntichainAddCell(ret, IntSetUnion(c1,(IntSet)l2->data));
          l2 = l2->next;
        }

      l1 = l1->next;
    }

  AntichainRelease(atc1);
  AntichainRelease(atc2);
  return ret;
}

#ifdef MINISAT
Antichain RemoveUnsat_cpp (Antichain a)
{
  guint i;
  guint j;
  guint* max_id_var = g_new(guint, a->mng->nbr_clauses);

  Antichain ret = g_new(AntichainStruct, 1);
  ret->mng = a->mng;
  ret->blocs = NULL;
  GSList* l = a->blocs;
  while(l)
    {
      vec<Lit> lits;
      Solver S;
      S.newVar();


      GUIntList* elems_in_bloc = a->mng->elems_in_bloc;
      while(elems_in_bloc)
        {
          if(IntSetHasElem((IntSet) l->data,elems_in_bloc->data))
            {
              lits.clear();
              GUIntList* recu = a->mng->desc_clauses[elems_in_bloc->data];
              while(recu)
                {
                  guint id_elem = recu->data / 2;
                  while (id_elem >= S.nVars()) 
                    S.newVar();
                  
                  if(recu->data & one) //impair
                    lits.push(Lit(id_elem));
                  else
                    lits.push(~Lit(id_elem));
                  recu = recu->next;
                }
          S.addClause(lits);
            }
          elems_in_bloc = elems_in_bloc->next;
        }
      bool is_sat = S.solve();
      if (is_sat)
        {
          ret->blocs = g_slist_prepend(ret->blocs, l->data);
        }
      else
        {
          IntSetRelease((IntSet)l->data);
        }
       l = l->next;
    }
  AntichainRelease(a);
  return ret;
}

Antichain RemoveUnsat (Antichain a)
{
  return RemoveUnsat_cpp(a);
}
#endif

extern "C" {
  gboolean IsTrivial(Antichain a)
  {
    #ifdef STOPTEST
    if (a->blocs == NULL 
        || (a->blocs->next == NULL && IntSetIsEmpty((IntSet)a->blocs->data)))
      return true;
    else
      return false;
    #else
    return false;
    #endif
  }
}

extern "C" {
  Antichain PreForAll(Antichain a, glong lit)
  {
  Antichain pre_0 = g_new(AntichainStruct, 1);
  pre_0->mng = a->mng;
  pre_0->blocs = NULL;
  Antichain pre_1 = g_new(AntichainStruct, 1);
  pre_1->mng = a->mng;
  pre_1->blocs = NULL;
  GSList* l = a->blocs;

  #if defined(MINISAT) || defined(OPTIMA)
  UpdateClauseLit(a->mng,lit);
  #endif

  while(l)
    {
      IntSet d = (IntSet)l->data;
      AntichainAddCell(pre_0,CellPre(a->mng,(IntSet)l->data,lit));
      AntichainAddCell(pre_1,CellPre(a->mng,(IntSet)l->data,lit +1));
      IntSetRelease((IntSet)l->data);
      l = l->next;
    }

  Antichain ret;

  #ifdef STOPTEST
  if(pre_0->blocs == NULL || pre_1->blocs == NULL)
    {
      ret = EmptyAntichain(pre_0->mng);
      AntichainRelease(pre_0);
      AntichainRelease(pre_1);
    }
  else if(IsTrivial(pre_0)) //pre_0 est l'antichain avec que des 000
    {
      AntichainRelease(pre_0);
      
      #ifdef MINISAT
      pre_1 = RemoveUnsat(pre_1);
      #endif          
      ret = pre_1;
    }
  else if (IsTrivial(pre_1)) //pre_1 est l'antichain avec que des 000
    {
      AntichainRelease(pre_1);
      
      #ifdef MINISAT
      pre_0 = RemoveUnsat(pre_0);
      #endif          
      ret = pre_0;
    }
  else
    {
      #ifdef MINISAT
      pre_0 = RemoveUnsat(pre_0);
      pre_1 = RemoveUnsat(pre_1);
      #endif
      ret =  (AntichainCrossUnion(pre_0,pre_1));
    }
  #else
    #ifdef MINISAT
    pre_0 = RemoveUnsat(pre_0);
    pre_1 = RemoveUnsat(pre_1);
    #endif
    ret =  (AntichainCrossUnion(pre_0,pre_1));
  #endif

  #ifdef MINISAT
  UpdateElemsInBloc(ret);
  ret = RemoveUnsat (ret);
  #endif

  return ret;
  }
}

extern "C" {
  Antichain PreExists(Antichain a, glong lit)
  {
  Antichain ret = g_new(AntichainStruct, 1);
  ret->mng = a->mng;
  ret->blocs = NULL;
  GSList* l = a->blocs;
  
  #if defined(MINISAT) || defined(OPTIMA)
  UpdateClauseLit(a->mng,lit); // this action is perform here,
  // because the action doesn't modify lit_clauses (used by CellPre)
  // and if we want a good work of RemoveSuperSetClauses this action
  // must be performed.
  #endif
  
  while(l)
    {
      AntichainAddCell(ret,CellPre(a->mng,(IntSet)l->data,lit +1));
      AntichainAddCell(ret,CellPre(a->mng,(IntSet)l->data,lit));
      
      IntSetRelease((IntSet)l->data);
      l = l->next;
    }

  #ifdef MINISAT
  UpdateElemsInBloc(ret);
  ret = RemoveUnsat (ret);
  #endif

  return ret;
  }
}

