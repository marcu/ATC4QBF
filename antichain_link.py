from ctypes import *
import os

class IntSetManagerStruct (Structure) :
    _fields_ = [("N",c_uint),
                ("last_only_one", c_longlong),
                ("nbr_guint64_IntSet", c_uint)]

class IntSetStruct(Structure) : 
    _fields_ = [("mng", POINTER(IntSetManagerStruct)),
                ("set", c_longlong)]

class AntichainManagerStruct(Structure) :
    pass

class AntichainStruct(Structure) :
    pass

class PBM(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

class TBD(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

if (os.uname())[0] == 'Darwin' : # mac-os
    lib_atc = cdll.LoadLibrary("antichain.dylib")
elif (os.uname())[0] == 'Linux' : # linux
    lib_atc = cdll.LoadLibrary("antichain.so")
else :
    raise TBD("windows dll")

"""
InitIntSetManager = lib_atc.InitIntSetManager
InitIntSetManager.restype = POINTER(IntSetManagerStruct)
InitIntSetManager.argtypes = [c_uint]

IntSetRelease = lib_atc.IntSetRelease
IntSetRelease.restype = c_void_p
IntSetRelease.argtypes = [POINTER(IntSetStruct)]

IntSetEmpty = lib_atc.IntSetEmpty
IntSetEmpty.restype = POINTER(IntSetStruct)
IntSetEmpty.argtypes = [POINTER(IntSetManagerStruct)]

IntSetFull = lib_atc.IntSetFull
IntSetFull.restype = POINTER(IntSetStruct)
IntSetFull.argtypes = [POINTER(IntSetManagerStruct)]

IntSetPrint = lib_atc.IntSetPrint
IntSetPrint.restype =  c_void_p
IntSetPrint.argtypes = [POINTER(IntSetStruct)]

IntSetCompare = lib_atc.IntSetCompare
IntSetCompare.restype = c_int
IntSetCompare.argtypes = [POINTER(IntSetStruct), POINTER(IntSetStruct)]

IntSetIsEqual = lib_atc.IntSetIsEqual
IntSetIsEqual.restype = c_long
IntSetIsEqual.argtypes = [POINTER(IntSetStruct), POINTER(IntSetStruct)]

IntSetIsEmpty = lib_atc.IntSetIsEmpty
IntSetIsEmpty.restype = c_long
IntSetIsEmpty.argtypes = [POINTER(IntSetStruct)]

IntSetUnion = lib_atc.IntSetUnion
IntSetUnion.restype = POINTER(IntSetStruct)
IntSetUnion.argtypes = [POINTER(IntSetStruct), POINTER(IntSetStruct)]

IntSetExcUnion = lib_atc.IntSetExcUnion
IntSetExcUnion.restype = POINTER(IntSetStruct)
IntSetExcUnion.argtypes = [POINTER(IntSetStruct), POINTER(IntSetStruct)]

IntSetIntersection = lib_atc.IntSetIntersection
IntSetIntersection.restype = POINTER(IntSetStruct)
IntSetIntersection.argtypes = [POINTER(IntSetStruct), POINTER(IntSetStruct)]

IntSetComplement = lib_atc.IntSetComplement
IntSetComplement.restype = POINTER(IntSetStruct)
IntSetComplement.argtypes = [POINTER(IntSetStruct)]

IntSetCopy = lib_atc.IntSetCopy
IntSetCopy.restype = POINTER(IntSetStruct)
IntSetCopy.argtypes = [POINTER(IntSetStruct)]

IntSetAddElem = lib_atc.IntSetAddElem
IntSetAddElem.restype = POINTER(IntSetStruct)
IntSetAddElem.argtypes = [POINTER(IntSetStruct), c_long]

IntSetRmElem = lib_atc.IntSetRmElem
IntSetRmElem.restype = POINTER(IntSetStruct)
IntSetRmElem.argtypes = [POINTER(IntSetStruct), c_long]

IntSetHasElem = lib_atc.IntSetHasElem
IntSetHasElem.restype = c_long
IntSetHasElem.argtypes = [POINTER(IntSetStruct), c_long]

IntSetIsInclude = lib_atc.IntSetIsInclude
IntSetIsInclude.restype = c_long
IntSetIsInclude.argtypes = [POINTER(IntSetStruct), POINTER(IntSetStruct)]

"""
InitAntichainManager = lib_atc.InitAntichainManager
InitAntichainManager.restype = POINTER(AntichainManagerStruct)
InitAntichainManager.argtypes = [c_uint, c_uint]

InitLitClause = lib_atc.InitLitClause
InitLitClause.restype = c_void_p
InitLitClause.argtypes = [POINTER(AntichainManagerStruct), c_uint, c_uint]

InitAntichain = lib_atc.InitAntichain
InitAntichain.restype = POINTER(AntichainStruct)
InitAntichain.argtypes = [POINTER(AntichainManagerStruct)]

PreForAll = lib_atc.PreForAll
PreForAll.restype = POINTER(AntichainStruct)
PreForAll.argtypes = [POINTER(AntichainStruct), c_long]

PreExists = lib_atc.PreExists
PreExists.restype = POINTER(AntichainStruct)
PreExists.argtypes = [POINTER(AntichainStruct), c_long]

AntichainPrint = lib_atc.AntichainPrint
AntichainPrint.restype = c_void_p
AntichainPrint.argtypes = [POINTER(AntichainStruct)]

AntichainLength = lib_atc.AntichainLength
AntichainLength.restype = c_uint
AntichainLength.argtypes = [POINTER(AntichainStruct)]

AntichainManagerPrint = lib_atc.AntichainManagerPrint
AntichainManagerPrint.restype = c_void_p
AntichainManagerPrint.argtypes = [POINTER(AntichainManagerStruct)]

IsTrivial = lib_atc.IsTrivial
IsTrivial.restype = c_long
IsTrivial.argtypes = [POINTER(AntichainStruct)]

def get_id_lit (x) :
    if x > 0 :
        return (x - 1) * 2
    elif x < 0 :
        return (((x + 1) * (-1)) * 2) + 1
    else :
        raise PBM("0 dans get_id_lit")
        

def get_id_lit_ae (x) :
    if x > 0 :
        return get_id_lit(x)
    elif x < 0 :
        return get_id_lit(x * (-1))
    else :
        raise PBM("0 dans get_id_lit_ae")
