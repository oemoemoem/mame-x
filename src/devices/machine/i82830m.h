// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
// Intel i82830M northbridge

#ifndef MAME_MACHINE_I82830M_H
#define MAME_MACHINE_I82830M_H

#pragma once

#include "pci.h"
#include "video/pc_vga.h"

class i82830m_host_device : public pci_host_device {
public:
	template <typename T>
	i82830m_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t subdevice_id, T &&cpu_tag, int ram_size)
		: i82830m_host_device(mconfig, tag, owner, clock)
	{
		set_ids_host(0x80863575, 0x02, subdevice_id);
		set_cpu_tag(std::forward<T>(cpu_tag));
		set_ram_size(ram_size);
	}

	i82830m_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_cpu_tag(T &&tag) { cpu.set_tag(std::forward<T>(tag)); }
	void set_ram_size(int ram_size);

	virtual uint8_t capptr_r() override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void reset_all_mappings() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override;

private:
	void agp_translation_map(address_map &map);

	int ram_size;
	required_device<device_memory_interface> cpu;
	std::vector<uint32_t> ram;

	uint8_t agpm, fpllcont, pam[8], smram, esmramc;
	uint8_t apsize, amtt, lptt;
	uint16_t toud, mchcfg, errcmd, smicmd, scicmd, skpd;
	uint32_t agpctrl, attbase;

	uint8_t agpm_r();
	void agpm_w(uint8_t data);
	uint8_t gc_r();
	uint8_t csabcont_r();
	uint32_t eap_r();
	uint8_t derrsyn_r();
	uint8_t des_r();
	uint8_t fpllcont_r();
	void fpllcont_w(uint8_t data);
	uint8_t pam_r(offs_t offset);
	void pam_w(offs_t offset, uint8_t data);
	uint8_t smram_r();
	void smram_w(uint8_t data);
	uint8_t esmramc_r();
	void esmramc_w(uint8_t data);
	uint32_t acapid_r();
	uint32_t agpstat_r();
	uint32_t agpcmd_r();
	uint32_t agpctrl_r();
	void agpctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t apsize_r();
	void apsize_w(uint8_t data);
	uint32_t attbase_r();
	void attbase_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t amtt_r();
	void amtt_w(uint8_t data);
	uint8_t lptt_r();
	void lptt_w(uint8_t data);
	uint16_t toud_r();
	void toud_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t mchcfg_r();
	void mchcfg_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t errsts_r();
	uint16_t errcmd_r();
	void errcmd_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t smicmd_r();
	void smicmd_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t scicmd_r();
	void scicmd_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t skpd_r();
	void skpd_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t capreg1_r();
	uint8_t capreg2_r();
};

class i82830m_agp_device : public agp_bridge_device {
public:
	i82830m_agp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

class i82830m_vga_device : public pci_device {
public:
	i82830m_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void legacy_memory_map(address_map &map);
	void legacy_io_map(address_map &map);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	u32 gmadr_r();
	virtual u8 capptr_r() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override;

private:
	required_device<vga_device> m_vga;

	u8 vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);
	u32 vga_3b0_r(offs_t offset, uint32_t mem_mask = ~0);
	void vga_3b0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	u32 vga_3c0_r(offs_t offset, uint32_t mem_mask = ~0);
	void vga_3c0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	u32 vga_3d0_r(offs_t offset, uint32_t mem_mask = ~0);
	void vga_3d0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
};

DECLARE_DEVICE_TYPE(I82830M_HOST,     i82830m_host_device)
DECLARE_DEVICE_TYPE(I82830M_AGP,      i82830m_agp_device)
DECLARE_DEVICE_TYPE(I82830M_VGA,      i82830m_vga_device)

#endif // MAME_MACHINE_I82830M_H
