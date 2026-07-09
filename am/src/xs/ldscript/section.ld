ENTRY(_start)

SECTIONS {
  . = ORIGIN(ram);
  .text : {
    *(entry)
    *(.text)
  }
  etext = .;
  _etext = .;
  .rodata : {
    *(.rodata*)
  }
  .data : {
    *(.data)
  }
  edata = .;
  _data = .;
  .bss : {
	_bss_start = .;
    *(.bss*)
    *(.sbss*)
    *(.scommon)
  }

  /* ECC 测试数据段：与关键变量隔离，避免错误注入影响程序逻辑 */
  .ecc_test_data : {
    *(.ecc_test_data)
  }
  _ecc_test_data_end = .;

  /* stack && TLS, 128KB each */
  _stack_top = ALIGN(0x1000);
  . = _stack_top + 0x40000;
  _stack_pointer = .;

  /* thread-local data segment */
  .tdata :
  {
    _tdata_begin = .;
    *(.tdata)
    _tdata_end = .;
  }
  .tbss :
  {
    *(.tbss)
    _tbss_end = .;
  }

  end = .;
  _end = .;
  _heap_start = ALIGN(0x1000);
  _pmem_start = pmem_base;
  _pmem_end = _pmem_start + LENGTH(ram);
}
