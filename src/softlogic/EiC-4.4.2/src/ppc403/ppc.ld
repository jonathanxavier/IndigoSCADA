/*
 *  This file contains directives for the GNU linker which are specific
 *  to the Papyrus.
 *
 *  linkcmds,v 1.2 1995/12/19 20:10:38 joel Exp
 */

OUTPUT_FORMAT("elf32-powerpc", "elf32-powerpc",
              "elf32-powerpc")
OUTPUT_ARCH(powerpc)
SEARCH_DIR(/ppc);
STARTUP(ppc403/io.o) 
ENTRY(_start) 
 
MEMORY
  {
        RAM : ORIGIN = 0x7fe00000, LENGTH = 512K
        ROM : ORIGIN = 0x7ff10000, LENGTH = 384K
  }
 
/* Do we need any of these for elf?
   __DYNAMIC = 0;    */
SECTIONS
{
/* .vectors 0x0100 :
  {
    *(.vectors)
  }
*/ 
  .text : /* code location */
  {
     *(.entry)
     *(.entry2)
     *(.text)
     *(.rodata)
     *(.rodata1)
  } > RAM

 
  /* R/W Data */
  .data :
  {
    *(.descriptors)
    *(rom_ver)
    *(.sdata)
    *(.sdata2)
    *(.got2)
    *(.fixup)

    *(.data)
    *(.data1)
  } > RAM
 
  .bss :
  {
    bss.start = .;
    *(.bss) *(.sbss) *(COMMON)
    bss.end = ALIGN(4);
    s.got = .;
    *(.got.plt) *(.got)
  } > RAM
 
  bss.size = bss.end - bss.start;
  PROVIDE(_end = bss.end);

  .line 0 : { *(.line) }
  .debug 0 : { *(.debug) }
  .debug_sfnames 0 : { *(.debug_sfnames) }
  .debug_srcinfo 0 : { *(.debug_srcinfo) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  .debug_aranges 0 : { *(.debug_aranges) }
  .debug_aregion 0 : { *(.debug_aregion) }
  .debug_macinfo 0 : { *(.debug_macinfo) }
  .stab 0 : { *(.stab) }
  .stabstr 0 : { *(.stabstr) }
}


