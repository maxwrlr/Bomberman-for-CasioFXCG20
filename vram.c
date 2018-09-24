void *GetVRAMAddress() {
	__asm__("mov.l syscall_adress, r2\n"
			"mov.l getVRAM, r0\n"
			"jmp @r2\n"
			"nop\n"
			"syscall_adress: .long 0x80020070\n"
			"getVRAM: .long 0x01E6");
}
//

