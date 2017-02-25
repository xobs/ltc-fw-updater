/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio.

    This file is part of ChibiOS.

    ChibiOS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * KL25Z128 memory setup.
 */

ENTRY(Esplanade_Main)

MEMORY
{
  osflash  :  org = 0x00000000,                  len = 0x5900
  flash    :  org = LENGTH(osflash),             len = 32k - LENGTH(osflash)
  osram    :  org = 0x1FFFFC00,                  len = 1024
  ram      :  org = 0x1FFFFC00 + LENGTH(osram),  len = 4k - LENGTH(osram)
}

/* Make the RAM size available to the OS */
__ram_start__           = ORIGIN(ram);
__ram_size__            = LENGTH(ram);
__ram_end__             = __ram_start__ + __ram_size__;

__os_storage_start__    = ORIGIN(osflash);
__os_storage_size__     = LENGTH(osflash);
__os_storage_end__      = __os_storage_start__ + __os_storage_size__ - 1;

SECTIONS
{
  . = ORIGIN(flash);

  .text : ALIGN(4) SUBALIGN(4)
  {
    KEEP(*(.progheader))

    _text = .;

    PROVIDE(__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array))
    PROVIDE(__init_array_end = .);

    *(.text .stub .text.* .gnu.linkonce.t.*)
    KEEP(*(SORT(.chibi_list*)));
    *(.rodata)
    *(.rodata.*)

    *(.ARM.extab* .gnu.linkonce.armextab.*)

    PROVIDE(__exidx_start = .);
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    PROVIDE(__exidx_end = .);

    *(.eh_frame_hdr)

    *(.eh_frame)
  } > flash

  /* Special section for exceptions stack.*/
  .mstack :
  {
      . = ALIGN(8);
      __main_stack_base__ = .;
      . += __main_stack_size__;
      . = ALIGN(8);
      __main_stack_end__ = .;
  } > ram

  /* Special section for process stack.*/
  .pstack :
  {
      __process_stack_base__ = .;
      . += __process_stack_size__;
      . = ALIGN(8);
      __process_stack_end__ = .;
  } > ram

  .data :
  {
    . = ALIGN(4);
    PROVIDE(_data_start = .);
    PROVIDE(_data_flash_start = LOADADDR(.data));
    *(.data .data.* .gnu.linkonce.d.*)
    . = ALIGN(4);
    PROVIDE(_data_end = .);
  } > ram AT > flash

  .ramtext : ALIGN(4)
  {
    PROVIDE(_ramtext_flash_start = LOADADDR(.ramtext));
    PROVIDE(_ramtext_start = .);
    *(.ramtext)
    . = ALIGN(4);
    PROVIDE(_ramtext_end = .);
  } > osram AT > flash

  .os_storage (NOLOAD) : ALIGN(1024)
  {
    *(.os_storage)
  } > flash

  /* These two hold temporary copies of page 0 and page 1,
   * which must be programmed after the rest of the program
   * has finished loading.
   */
  .page0_storage (NOLOAD) : ALIGN(1024)
  {
    *(.page0_storage)
  } > flash

  .page1_storage (NOLOAD) : ALIGN(1024)
  {
    *(.page1_storage)
  } > flash

  .bss :
  {
    . = ALIGN(4);
    PROVIDE(_bss_start = .);
    *(.bss .bss.* .gnu.linkonce.b.*)
    . = ALIGN(4);
    *(COMMON)
    . = ALIGN(4);
    PROVIDE(_bss_end = .);
  } > ram

  PROVIDE(end = .);
  _end            = .;

  /DISCARD/ : {
    *(.note.GNU-stack)
    *(.gnu_debuglink)
    *(.note.gnu.build-id)
    *(.note.gnu.build-id.*);
    *(.glue_7t)
    *(.glue_7)
    *(.gcc*)
    *(.interp)
    *(.fini_array)
    *(.fini_array.*)
  }

  /* ld.bfd will segfault without these */
  .rel.plt :
  {
    . = ALIGN(4);
  }
  .got.plt :
  {
    . = ALIGN(4);
  }
}

/* RAM region to be used for Main stack. This stack accommodates the processing
      of all exceptions and interrupts*/
REGION_ALIAS("MAIN_STACK_RAM", ram);

/* RAM region to be used for the process stack. This is the stack used by
      the main() function.*/
REGION_ALIAS("PROCESS_STACK_RAM", ram);

/* RAM region to be used for data segment.*/
REGION_ALIAS("DATA_RAM", ram);

/* RAM region to be used for BSS segment.*/
REGION_ALIAS("BSS_RAM", ram);

__heap_base__   = _end;
__heap_end__    = __ram_end__;
