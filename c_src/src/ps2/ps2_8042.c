#include "ps2_8042.h"
#include "printer.h"

status_register_t get_statusreg() {
  status_register_t stat = PS2_STATUS();
  tracek("%d%d%d%d%d%d\n", stat.output_full, stat.input_full, stat.sys_flag,
         stat.to_controller, stat.err_timeout, stat.err_parity);

  return stat;
};

void init_PS2_8042() {

  // Disable port 1 and 2
  PS2_COMMAND(0xAD);
  PS2_COMMAND(0xA7);

  // flush data buffer
  PS2_RX();

  // request Controller Configuration Byte
  PS2_COMMAND(0x20);
  controller_configuration_byte_t ccb =
      (controller_configuration_byte_t)PS2_RX();

  // tweak it
  ccb.disable_clk_p1 = 0;
  ccb.disable_clk_p2 = 1;
  ccb.en_int_p1 = 1;
  ccb.en_int_p2 = 0;
  ccb.en_p1_translation = 0; // important!

  // and update it
  PS2_COMMAND(0x60);
  PS2_TX(ccb.raw);

  // verify Controller Configuration Byte
  PS2_COMMAND(0x20);

  if (PS2_RX() != ccb.raw) {
    printk("could not write to ccb\n");
    asm("hlt");
  }

  // enable port 1
  PS2_COMMAND(0xAE);
};
