// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_VIDEO_GF7600GS_H
#define MAME_VIDEO_GF7600GS_H

#pragma once

#include "machine/pci.h"
#include "video/pc_vga_nvidia.h"
#include "video/nvidia_curie.h"

class geforce_7600gs_device : public agp_device {
public:
	template <typename T, typename U> geforce_7600gs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t subdevice_id, T &&vga_tag, U &&cpu_tag)
		: geforce_7600gs_device(mconfig, tag, owner, clock)
	{
		set_ids_agp(0x10de02e1, 0xa1, subdevice_id);
		m_vga.set_tag(std::forward<T>(vga_tag));
		set_cpu_tag(std::forward<U>(cpu_tag));
	}
	geforce_7600gs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template <typename T> void set_cpu_tag(T &&cpu_tag) { cpu.set_tag(std::forward<T>(cpu_tag)); }
	nv40_renderer *debug_get_renderer() { return nvidia_nv40; }

	void legacy_memory_map(address_map &map);
	void legacy_io_map(address_map &map);
	
	uint32_t geforce_r(offs_t offset, uint32_t mem_mask = ~0);
	void geforce_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t nv40_mirror_r(offs_t offset, uint32_t mem_mask = ~0);
	void nv40_mirror_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override;

private:
	required_device<nvidia_g73_vga_device> m_vga;
	nv40_renderer *nvidia_nv40;
	required_device<device_memory_interface> cpu;
	address_space *m_program;

	u8 vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);
	u32 vga_3b0_r(offs_t offset, uint32_t mem_mask = ~0);
	void vga_3b0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	u32 vga_3c0_r(offs_t offset, uint32_t mem_mask = ~0);
	void vga_3c0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	u32 vga_3d0_r(offs_t offset, uint32_t mem_mask = ~0);
	void vga_3d0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void map1(address_map &map);
	void map2(address_map &map);
	void map3(address_map &map);

	bool m_vga_legacy_enable = false;
	u32 m_main_scratchpad_id = 0;
};

DECLARE_DEVICE_TYPE(GEFORCE_7600GS, geforce_7600gs_device)

#endif // MAME_VIDEO_GF7600GS_H
