// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "pci-ac97.h"

#include "speaker.h"

DEFINE_DEVICE_TYPE(AC97, ac97_device, "ac97", "AC'97 Audio")
DEFINE_DEVICE_TYPE(MC97, mc97_device, "mc97", "MC'97 Audio")

void ac97_device::native_audio_mixer_map(address_map &map)
{
}

void ac97_device::native_audio_bus_mastering_map(address_map &map)
{
}

void ac97_device::mixer_map(address_map &map)
{
}

void ac97_device::bus_mastering_map(address_map &map)
{
}

ac97_device::ac97_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, AC97, tag, owner, clock)
{
}

void ac97_device::device_start()
{
	pci_device::device_start();
	add_map(256, M_IO, FUNC(ac97_device::native_audio_mixer_map));
	add_map(64,  M_IO, FUNC(ac97_device::native_audio_bus_mastering_map));
	add_map(512, M_MEM, FUNC(ac97_device::mixer_map));
	add_map(256, M_MEM, FUNC(ac97_device::bus_mastering_map));
}

void ac97_device::device_reset()
{
	pci_device::device_reset();
}

void ac97_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}


mc97_device::mc97_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, MC97, tag, owner, clock)
{
}

void mc97_device::device_start()
{
	pci_device::device_start();
}

void mc97_device::device_reset()
{
	pci_device::device_reset();
}