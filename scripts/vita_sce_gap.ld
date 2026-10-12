/* Reserve a virtual gap for vita-elf-create SCE metadata; no extra loaded section. */
SECTIONS { . = ALIGN(0x10000) + 0x10000; }
INSERT BEFORE .preinit_array;
