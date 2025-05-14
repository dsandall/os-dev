#break long_mode_start
#break init_PS2
#break run_tasks
#break ps2_rx_task
#break vga_task
#break hw_serial_init
#break hw_serial_task
#break serial_tx_handler
break exception_handler
#break asm_int_handler
#break doubleprint
break kernel_main
break ps2_onkeypressevent
#break generate_memory_map
#disp vga_ipc
#disp ps2_ipc

layout src
#break read_tag
#break parse_elf_section_headers
#break generate_memory_map
#break fiftytwo_card_pickup
#break makePage
#break testPageAllocator
#break makePage
break testPageAllocator_stresstest
break validate_and_coalesce

break yeetr
disp/x skibiditol
disp/x skibidi
disp/x skib

c
