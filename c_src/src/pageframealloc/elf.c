/*
 *  The second reference you need is the ELF format description. This tells you
 * the format of the ELF section header. The ELF section header contains
 * information on which parts of memory your kernel is currently using. It is
 * very important to avoid avoid allocating this kernel memory to any other
 * kernel tasks or user level processes.
 */
