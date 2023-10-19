// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "gf7600gs.h"

DEFINE_DEVICE_TYPE(GEFORCE_7600GS, geforce_7600gs_device, "geforce_7600gs", "NVIDIA GeForce 7600GS")

void geforce_7600gs_device::map1(address_map &map)
{
	map(0x00000000, 0x00000fff).lrw32(
		NAME([this] (offs_t offset) {
			machine().debug_break();
			return m_main_scratchpad_id;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_main_scratchpad_id);
		})
	);
	map(0x00000000, 0xd2000000).ram().rw(FUNC(geforce_7600gs_device::geforce_r), FUNC(geforce_7600gs_device::geforce_w));
}

void geforce_7600gs_device::map2(address_map &map)
{
	map(0x00000000, 0xe0000000).ram().rw(FUNC(geforce_7600gs_device::nv40_mirror_r), FUNC(geforce_7600gs_device::nv40_mirror_w));
}

void geforce_7600gs_device::map3(address_map &map)
{
}

geforce_7600gs_device::geforce_7600gs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: agp_device(mconfig, GEFORCE_7600GS, tag, owner, clock)
	, m_vga(*this, finder_base::DUMMY_TAG)
	, nvidia_nv40(nullptr)
	, cpu(*this, finder_base::DUMMY_TAG)
	, m_program(nullptr)
{
}

void geforce_7600gs_device::device_start()
{
	agp_device::device_start();
	add_map( 16*1024*1024, M_MEM, FUNC(geforce_7600gs_device::map1));
	add_map(256*1024*1024, M_MEM, FUNC(geforce_7600gs_device::map2));
	add_map( 16*1024*1024, M_MEM, FUNC(geforce_7600gs_device::map3));
	m_program = &cpu->space(AS_PROGRAM); // FIXME: isn't there a proper way to map stuff or do DMA via the PCI device interface?
	nvidia_nv40 = new nv40_renderer(machine());
	nvidia_nv40->start(m_program);
	add_rom_from_region();

	intr_pin = 1;
	save_item(NAME(m_vga_legacy_enable));
}

void geforce_7600gs_device::legacy_memory_map(address_map &map)
{
	map(0xa0000, 0xbffff).rw(FUNC(geforce_7600gs_device::vram_r), FUNC(geforce_7600gs_device::vram_w));
}

void geforce_7600gs_device::legacy_io_map(address_map &map)
{
	map(0x03b0, 0x03bf).rw(FUNC(geforce_7600gs_device::vga_3b0_r), FUNC(geforce_7600gs_device::vga_3b0_w));
	map(0x03c0, 0x03cf).rw(FUNC(geforce_7600gs_device::vga_3c0_r), FUNC(geforce_7600gs_device::vga_3c0_w));
	map(0x03d0, 0x03df).rw(FUNC(geforce_7600gs_device::vga_3d0_r), FUNC(geforce_7600gs_device::vga_3d0_w));
}

void geforce_7600gs_device::map_extra(
	uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
	uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space
)
{
	if (m_vga_legacy_enable)
	{
		memory_space->install_readwrite_handler(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(geforce_7600gs_device::vram_r)), write8sm_delegate(*this, FUNC(geforce_7600gs_device::vram_w)));

		io_space->install_readwrite_handler(0x3b0, 0x3bf, read32s_delegate(*this, FUNC(geforce_7600gs_device::vga_3b0_r)), write32s_delegate(*this, FUNC(geforce_7600gs_device::vga_3b0_w)));
		io_space->install_readwrite_handler(0x3c0, 0x3cf, read32s_delegate(*this, FUNC(geforce_7600gs_device::vga_3c0_r)), write32s_delegate(*this, FUNC(geforce_7600gs_device::vga_3c0_w)));
		io_space->install_readwrite_handler(0x3d0, 0x3df, read32s_delegate(*this, FUNC(geforce_7600gs_device::vga_3d0_r)), write32s_delegate(*this, FUNC(geforce_7600gs_device::vga_3d0_w)));
		//memory_space->install_rom(0xc0000, 0xcffff, (void *)expansion_rom);
	}
}

void geforce_7600gs_device::device_reset()
{
	agp_device::device_reset();
	nvidia_nv40->set_ram_base(m_program->get_read_ptr(0));

	status = 0x02b0;
	command = 0x0007;

	m_vga_legacy_enable = true;
	m_main_scratchpad_id = 0x04b100a1;
	remap_cb();
}

// TODO: counter-check everything
void geforce_7600gs_device::config_map(address_map &map)
{
	agp_device::config_map(map);
	map(0x34, 0x34).lr8(NAME([] () { return 0x44; }));

//  map(0x40, 0x43) subsystem ID alias (writeable)
//  map(0x44, 0x4f) AGP i/f
//  map(0x50, 0x53) ROM shadow enable
	map(0x54, 0x57).lrw8(
		NAME([this] (offs_t offset) { return m_vga_legacy_enable; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_0_7)
			{
				m_vga_legacy_enable = BIT(data, 0);
				remap_cb();
			}
			//printf("- %s VGA control %08x & %08x\n", m_vga_legacy_enable ? "Enable" : "Disable", data, mem_mask);
		})
	);
	map(0x58, 0x58).lr8(NAME([] () { return 0x00; }));

}

uint32_t geforce_7600gs_device::geforce_r(offs_t offset, uint32_t mem_mask)
{
	return nvidia_nv40->geforce_r(offset, mem_mask);
}

void geforce_7600gs_device::geforce_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	nvidia_nv40->geforce_w(space, offset, data, mem_mask);
}

uint32_t geforce_7600gs_device::nv40_mirror_r(offs_t offset, uint32_t mem_mask)
{
	return m_program->read_dword(offset << 2);
}

void geforce_7600gs_device::nv40_mirror_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_program->write_dword(offset << 2, data, mem_mask);
}

uint8_t geforce_7600gs_device::vram_r(offs_t offset)
{
	return downcast<nvidia_g73_vga_device *>(m_vga.target())->mem_r(offset);
}

void geforce_7600gs_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<nvidia_g73_vga_device *>(m_vga.target())->mem_w(offset, data);
}

u32 geforce_7600gs_device::vga_3b0_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;
	if (ACCESSING_BITS_0_7)
		result |= downcast<nvidia_g73_vga_device *>(m_vga.target())->port_03b0_r(offset * 4 + 0) << 0;
	if (ACCESSING_BITS_8_15)
		result |= downcast<nvidia_g73_vga_device *>(m_vga.target())->port_03b0_r(offset * 4 + 1) << 8;
	if (ACCESSING_BITS_16_23)
		result |= downcast<nvidia_g73_vga_device *>(m_vga.target())->port_03b0_r(offset * 4 + 2) << 16;
	if (ACCESSING_BITS_24_31)
		result |= downcast<nvidia_g73_vga_device *>(m_vga.target())->port_03b0_r(offset * 4 + 3) << 24;
	return result;
}

void geforce_7600gs_device::vga_3b0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		downcast<nvidia_g73_vga_device *>(m_vga.target())->port_03b0_w(offset * 4 + 0, data >> 0);
	if (ACCESSING_BITS_8_15)
		downcast<nvidia_g73_vga_device *>(m_vga.target())->port_03b0_w(offset * 4 + 1, data >> 8);
	if (ACCESSING_BITS_16_23)
		downcast<nvidia_g73_vga_device *>(m_vga.target())->port_03b0_w(offset * 4 + 2, data >> 16);
	if (ACCESSING_BITS_24_31)
		downcast<nvidia_g73_vga_device *>(m_vga.target())->port_03b0_w(offset * 4 + 3, data >> 24);
}


u32 geforce_7600gs_device::vga_3c0_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;
	if (ACCESSING_BITS_0_7)
		result |= downcast<nvidia_g73_vga_device *>(m_vga.target())->port_03c0_r(offset * 4 + 0) << 0;
	if (ACCESSING_BITS_8_15)
		result |= downcast<nvidia_g73_vga_device *>(m_vga.target())->port_03c0_r(offset * 4 + 1) << 8;
	if (ACCESSING_BITS_16_23)
		result |= downcast<nvidia_g73_vga_device *>(m_vga.target())->port_03c0_r(offset * 4 + 2) << 16;
	if (ACCESSING_BITS_24_31)
		result |= downcast<nvidia_g73_vga_device *>(m_vga.target())->port_03c0_r(offset * 4 + 3) << 24;
	return result;
}

void geforce_7600gs_device::vga_3c0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		downcast<nvidia_g73_vga_device *>(m_vga.target())->port_03c0_w(offset * 4 + 0, data >> 0);
	if (ACCESSING_BITS_8_15)
		downcast<nvidia_g73_vga_device *>(m_vga.target())->port_03c0_w(offset * 4 + 1, data >> 8);
	if (ACCESSING_BITS_16_23)
		downcast<nvidia_g73_vga_device *>(m_vga.target())->port_03c0_w(offset * 4 + 2, data >> 16);
	if (ACCESSING_BITS_24_31)
		downcast<nvidia_g73_vga_device *>(m_vga.target())->port_03c0_w(offset * 4 + 3, data >> 24);
}

u32 geforce_7600gs_device::vga_3d0_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;
	if (ACCESSING_BITS_0_7)
		result |= downcast<nvidia_g73_vga_device *>(m_vga.target())->port_03d0_r(offset * 4 + 0) << 0;
	if (ACCESSING_BITS_8_15)
		result |= downcast<nvidia_g73_vga_device *>(m_vga.target())->port_03d0_r(offset * 4 + 1) << 8;
	if (ACCESSING_BITS_16_23)
		result |= downcast<nvidia_g73_vga_device *>(m_vga.target())->port_03d0_r(offset * 4 + 2) << 16;
	if (ACCESSING_BITS_24_31)
		result |= downcast<nvidia_g73_vga_device *>(m_vga.target())->port_03d0_r(offset * 4 + 3) << 24;
	return result;
}

void geforce_7600gs_device::vga_3d0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		downcast<nvidia_g73_vga_device *>(m_vga.target())->port_03d0_w(offset * 4 + 0, data >> 0);
	if (ACCESSING_BITS_8_15)
		downcast<nvidia_g73_vga_device *>(m_vga.target())->port_03d0_w(offset * 4 + 1, data >> 8);
	if (ACCESSING_BITS_16_23)
		downcast<nvidia_g73_vga_device *>(m_vga.target())->port_03d0_w(offset * 4 + 2, data >> 16);
	if (ACCESSING_BITS_24_31)
		downcast<nvidia_g73_vga_device *>(m_vga.target())->port_03d0_w(offset * 4 + 3, data >> 24);
}