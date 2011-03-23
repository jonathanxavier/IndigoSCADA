/* MachSet.h
 *
 *	(C) Copyright May  7 1995, Edmond J. Breen.
 *		   ALL RIGHTS RESERVED.
 * This code may be copied for personal, non-profit use only.
 *
 */

typedef enum { /* Stack Machine Instruction Set */
  halt=400,

  bump,

  /* If jumps are altered, added to or rearranged,
   * then check with the routine analyseCode -
   * in file optomizer.c
   */
  jmptab, jmpu,
  jmpFint, jmpFlng,  jmpFdbl, jmpFptr,
  jmpTint, jmpTlng,  jmpTdbl, jmpTptr,

  dupval,dupdown,
  lval,

  ixa,

  incdouble, decdouble,
  stodouble, rvaldouble, pushdouble,
  drefdouble, refdouble,
  negdouble, adddouble, subdouble, divdouble, multdouble,
  ltdouble, ledouble, eqdouble, nedouble, gtdouble,
  gedouble, lordouble, landdouble, notdouble,
  double2char, double2int, double2long, double2float,

  incint, decint,
  stoint, rvalint, pushint,
  drefint, refint,
  negint, addint, subint, divint, multint,
  ltint, leint, eqint, neint, gtint,
  geint, lorint, landint, notint, andint, borint,
  xorint, modint, lshtint, rshtint, compint,
  int2uchar, int2ushort, int2ptr, int2double,
  int2long,

  incuint, decuint,
  stouint, rvaluint, pushuint,
  drefuint, refuint,
  neguint, adduint, subuint, divuint, multuint,
  ltuint, leuint, equint, neuint, gtuint,
  geuint, loruint, notuint, anduint, boruint,
  xoruint, moduint, lshtuint, rshtuint, compuint,
  uint2char, uint2ptr, uint2double,
  uint2long,

  inclong, declong,
  stolong, rvallong, pushlong,
  dreflong, reflong,
  neglong, addlong, sublong, divlong, multlong,
  ltlong, lelong, eqlong, nelong, gtlong,
  gelong, lorlong, landlong, notlong, andlong, borlong,
  xorlong, modlong, lshtlong, rshtlong, complong,
  long2char, long2int, long2ptr, long2double,

  inculong, deculong,
  stoulong, rvalulong, pushulong,
  drefulong, refulong,
  negulong, addulong, subulong, divulong, multulong,
  ltulong, leulong, equlong, neulong, gtulong,
  geulong, lorulong, notulong, andulong, borulong,
  xorulong, modulong, lshtulong, rshtulong, compulong,
  ulong2char, ulong2int, ulong2ptr, ulong2double,

  drefptr, refptr, incptr, decptr,
  stoptr,   rvalptr,   pushptr,
  addptr2int, addint2ptr, subptrint,
  subptr, divptr,
  ltptr, leptr, eqptr, neptr, gtptr,
  geptr, lorptr, landptr, notptr,
  ptr2int, ptr2long,

  drefuchar, drefchar, refchar,  neguchar,
  rvalchar, stochar,
  rvaluchar,
  char2int, char2ptr, char2double,
  
  drefushort, drefshort, refshort,
  rvalshort, stoshort,
  rvalushort,
  
  dreffloat, reffloat,
  rvalfloat, stofloat,

  assigntype, stoval, pushval,
  massign,  fmem, mset, minit,
  refmem, reducear, checkar, checkar2,
  call, eiccall, endfunc, eicreturn, empty,

  lda
} MachSet_t;




