// license:BSD-3-Clause
// copyright-holders:aquaboxs
/***************************************************************************

    RCA RM4100 (MSN TV2) skeleton driver

***************************************************************************

RCA RM4100
Microsoft/RCA 2004-2013?

This is actual "PC-based" televsion system. Different CPU with Moblie Celeron and Pentium III.
Moblie Intel North/South-bridge and CF card slots etc.

On RCA RM4100 system, The CPU is a Moblie Celeron (Pentium III-based) at 733MHz. RAM is 128MB (not likely 256MB),
and CF card size is 64MB.

Mainboard
---------

   |--------|     |--------|  |---|     |-------|  |-------|  |-------|
   |INTERNET|     |ETHERNET|  |USB|     |       |  |       |  |       |   |-----|
---|        |-----|        |--|USB|-----|S-VIDEO|--|AUDIO-L|--|AUDIO-R|---|POWER|--
|																			      |
|                                                                                 |
|                                                                                 |
|                                                                                 |
|																			      |
|																				  |
|                                                  |--------------|               |
|                       |-------------|            |              |               |
|	|---------|         |             |            |              |               |
|	| 82801DB |         |    82830M   |            |              |               |
|	|		  |         |    SL62D    |            |      CPU     |               |
|	|		  |         |             |            |              |               |
|	|  INTEL  |         |    INTEL    |            |              |               |
|	|---------|         |-------------|            |              |               |
|                                                  |--------------|               |
|       C                                                                         |
|       F																		  |
|       																		  |
|       S    																	  |
|       L                                                                         |
|       O                                                                         |
|       T                                                                         |
|                                                                                 |
|    |------------|                                                               |
|    |            |                                                               |
|    |            |                                                               |
|    | SUPERIO    |                                                               |
|    |------------|   SIMM1  SIMM2(not used)  SIMM3(not used)  SIMM4(not used)    |
|                                                                                 |
-----------------------------------------------------------------------------------
Notes:
		CPU - Moblie Celeron 733MHz 256k L2 cache, 133MHz FSB. (Coppermine)
		SIMM1 - 128MB PC133 SDRAM on board
		82801DB - Intel Southbridge IC
		82830M - Intel Northbridge IC
		SUPERIO - SMSC LPC47M192 Super I/O
		CF SLOT - Accepts a compact flash card. The card is required to boot the system.
				  CF CARD: 64 MB SanDisk SDCFJ-64 (upgradeable)

*/

#include "emu.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/null_modem.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/sun_kbd.h"
#include "bus/rs232/terminal.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"
#include "machine/i82830m.h"
#include "machine/i82801db.h"
#include "machine/pci-usb.h"
#include "machine/pci-apic.h"
#include "machine/pci-smbus.h"
#include "machine/lpc47m192.h"
#include "sound/pci-ac97.h"

#include "bus/isa/isa.h"

#include "imagedev/floppy.h"
#include "machine/8042kbdc.h"
#include "machine/ins8250.h"
#include "machine/intelfsh.h"
#include "machine/pc_lpt.h"
#include "machine/pci.h"
#include "machine/upd765.h"

#include "formats/naslite_dsk.h"
#include "formats/pc_dsk.h"

#include "xbox_pci.h"

class rm4100_superio_device : public device_t, public lpcbus_device_interface
{
public:
	rm4100_superio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void map_extra(address_space *memory_space, address_space *io_space) override;
	virtual uint32_t dma_transfer(int channel, dma_operation operation, dma_size size, uint32_t data) override;
	virtual void set_host(int device_index, lpcbus_host_interface *host) override;

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t read_rs232(offs_t offset);
	void write_rs232(offs_t offset, uint8_t data);

protected:
	virtual void device_start() override;

private:
	void internal_io_map(address_map &map);

	lpcbus_host_interface *lpchost = nullptr;
	int lpcindex = 0;
	address_space *memspace = nullptr;
	address_space *iospace = nullptr;
	bool configuration_mode;
	int index;
	int selected;
	uint8_t registers[16][256]{}; // 256 registers for up to 16 devices, registers 0-0x2f common to all
};

DEFINE_DEVICE_TYPE(RM4100_SUPERIO, rm4100_superio_device, "rm4100_superio", "RM4100 debug SuperIO")

rm4100_superio_device::rm4100_superio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, RM4100_SUPERIO, tag, owner, clock)
	, configuration_mode(false)
	, index(0)
	, selected(0)
{

}

void rm4100_superio_device::internal_io_map(address_map &map)
{
	map(0x002e, 0x002f).rw(FUNC(rm4100_superio_device::read), FUNC(rm4100_superio_device::write));
	map(0x03f8, 0x03ff).rw(FUNC(rm4100_superio_device::read_rs232), FUNC(rm4100_superio_device::write_rs232));
}

void rm4100_superio_device::map_extra(address_space *memory_space, address_space *io_space)
{
	memspace = memory_space;
	iospace = io_space;
	io_space->install_device(0, 0xffff, *this, &rm4100_superio_device::internal_io_map);
}

void rm4100_superio_device::set_host(int device_index, lpcbus_host_interface *host)
{
	lpchost = host;
	lpcindex = device_index;
}

uint32_t rm4100_superio_device::dma_transfer(int channel, dma_operation operation, dma_size size, uint32_t data)
{
	logerror("LPC dma transfer attempted on channel %d\n", channel);
	return 0;
}

void rm4100_superio_device::device_start()
{
	memset(registers, 0, sizeof(registers));
	registers[0][0x26] = 0x2e; // Configuration port address byte 0
}

uint8_t rm4100_superio_device::read(offs_t offset)
{
	if (configuration_mode == false)
		return 0;
	if (offset == 0) // index port 0x2e
		return index;
	if (offset == 1)
	{
		// data port 0x2f
		if (index < 0x30)
			return registers[0][index];
		return registers[selected][index];
	}
	return 0;
}

void rm4100_superio_device::write(offs_t offset, uint8_t data)
{
	if (configuration_mode == false)
	{
		// config port 0x2e
		if ((offset == 0) && (data == 0x55))
			configuration_mode = true;
		return;
	}
	if ((offset == 0) && (data == 0xaa))
	{
		// config port 0x2e
		configuration_mode = false;
		return;
	}
	if (offset == 0)
	{
		// index port 0x2e
		index = data;
	}
	if (offset == 1)
	{
		// data port 0x2f
		if (index < 0x30)
		{
			registers[0][index] = data;
			selected = registers[0][7];
		}
		else
		{
			registers[selected][index] = data;
			//if ((superiost.selected == 4) && (superiost.index == 0x30) && (data != 0))
			//  ; // add handlers 0x3f8- +7
		}
	}
}

uint8_t rm4100_superio_device::read_rs232(offs_t offset)
{
	if (offset == 5)
		return 0x20;
	return 0;
}

void rm4100_superio_device::write_rs232(offs_t offset, uint8_t data)
{
	if (offset == 0)
	{
		printf("%c", data);
	}
}

namespace {

class rm4100_state : public driver_device
{
public:
	rm4100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void rm4100(machine_config &config);

private:
	required_device<pentium3_device> m_maincpu;

	void rm4100_map(address_map &map);

	//static void superio_config(device_t *device);
};

void rm4100_state::rm4100_map(address_map &map)
{
	map.unmap_value_high();
}

void rm4100_state::rm4100(machine_config &config)
{
	PENTIUM3(config, m_maincpu, 733'333'333); // Actually a Mobile Celeron (733'333'333)
	m_maincpu->set_addrmap(AS_PROGRAM, &rm4100_state::rm4100_map);
	//m_maincpu->set_addrmap(AS_IO, &rm4100_state::rm4100_map_io);
	m_maincpu->set_irq_acknowledge_callback("pci:1f.0:pic_master", FUNC(pic8259_device::inta_cb));

	PCI_ROOT(config, "pci", 0);
	I82830M_HOST(config, "pci:00.0", 0, 0x00000000, "maincpu", 128*1024*1024);
	I82830M_AGP(config, "pci:01.0", 0);
	RM4100_SUPERIO(config, "pci:01.0:0", 0);
	I82830M_VGA(config, "pci:02.0", 0);
	USB_UHCI(config, "pci:1d.0", 0, 0x808624c2, 0x02, 0x00000000);
	USB_UHCI(config, "pci:1d.1", 0, 0x808624c4, 0x02, 0x00000000);
	USB_UHCI(config, "pci:1d.2", 0, 0x808624c7, 0x02, 0x00000000);
	USB_EHCI(config, "pci:1d.7", 0, 0x808624cd, 0x02, 0x00000000);
	PCI_BRIDGE(config, "pci:1e.0", 0, 0x8086244e, 0x0a);
	I82801DB_LPC(config, "pci:1f.0", 0, m_maincpu);
	LPC_ACPI(config, "pci:1f.0:acpi", 0);
	LPC_RTC(config, "pci:1f.0:rtc", 0);
	LPC_PIT(config, "pci:1f.0:pit", 0);
	i82801db_ide_device &ide(I82801DB_IDE(config, "pci:1f.1", 0, m_maincpu));
	ide.irq_pri().set("pci:1f.0:pic_slave", FUNC(pic8259_device::ir6_w));
	ide.irq_sec().set("pci:1f.0:pic_slave", FUNC(pic8259_device::ir7_w));
	SMBUS(config, "pci:1f.3", 0, 0x808624c3, 0x02, 0x00000000);
	AC97(config, "pci:1f.5", 0, 0x808624a5, 0x02, 0x00000000);
	MC97(config, "pci:1f.6", 0, 0x808624a6, 0x02, 0x00000000);
}

ROM_START(rm4100)
	ROM_REGION32_LE(0x100000, "pci:1f.0", 0) /* PC bios */
	// Coreboot (SeaBIOS/RM4100) version
  	ROM_SYSTEM_BIOS(0, "bios0", "Coreboot (RCA RM4100)")
  	ROMX_LOAD("coreboot.rom",  0x00000, 0x100000, CRC(5777759b) SHA1(48cc40168924897a0b2c0cdb0c5dd08f19711abd), ROM_BIOS(0) )
ROM_END

}

CONS( 2004, rm4100,      0,      0, rm4100,       0, rm4100_state, empty_init, "RCA",               "RM4100 MSNTV2 Receiver", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )