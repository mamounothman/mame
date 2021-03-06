// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
HotBlock board

Tetris with naughty bits

        ||||||||||||||||
+-------++++++++++++++++-------+
|                              |
|  YM2149 TESTSW               |
|                              |
|    62256 62256   6116 6116   |
|                              |
|    24mhz  TPC1020AFN 24c04a  |
|                              |
|                     PAL      |
| P8088-1 IC4 IC5 62256 62256  |
|                              |
+------------------------------+

330ohm resistor packs for colours


--

there are a variety of test modes which can be obtained
by resetting while holding down player 2 buttons

eeprom / backup data not hooked up ( 24c04a on port4 )

most sources say this is a game by Nics but I believe Nics
to be a company from Korea, this game is quite clearly a
Spanish game, we know for a fact that NIX are from Spain
so it could be by them instead



*/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class hotblock_state : public driver_device
{
public:
	hotblock_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_vram(*this, "vram")  { }

	void hotblock(machine_config &config);

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_vram;

	/* misc */
	int      m_port0;
	int      m_port4;

	/* memory */
	uint8_t    m_pal[0x10000];

	DECLARE_READ8_MEMBER(video_read);
	DECLARE_READ8_MEMBER(port4_r);
	DECLARE_WRITE8_MEMBER(port4_w);
	DECLARE_WRITE8_MEMBER(port0_w);
	DECLARE_WRITE8_MEMBER(video_write);

	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void hotblock_io(address_map &map);
	void hotblock_map(address_map &map);
};



READ8_MEMBER(hotblock_state::video_read)
{
	/* right?, anything else?? */
	if (m_port0 & 0x20) // port 0 = a8 e8 -- palette
	{
		return m_pal[offset];
	}
	else // port 0 = 88 c8
	{
		return m_vram[offset];
	}
}

/* port 4 is some kind of eeprom / storage .. used to store the scores */
READ8_MEMBER(hotblock_state::port4_r)
{
//  osd_printf_debug("port4_r\n");
	return 0x00;
}


WRITE8_MEMBER(hotblock_state::port4_w)
{
	m_port4 = data;
}



WRITE8_MEMBER(hotblock_state::port0_w)
{
	m_port0 = data;
}

WRITE8_MEMBER(hotblock_state::video_write)
{
	/* right?, anything else?? */
	if (m_port0 & 0x20) // port 0 = a8 e8 -- palette
	{
		m_pal[offset] = data;
	}
	else // port 0 = 88 c8
	{
		m_vram[offset] = data;
	}
}

void hotblock_state::hotblock_map(address_map &map)
{
	map(0x00000, 0x0ffff).ram();
	map(0x10000, 0x1ffff).rw(FUNC(hotblock_state::video_read), FUNC(hotblock_state::video_write)).share("vram");
	map(0x20000, 0xfffff).rom();
}

void hotblock_state::hotblock_io(address_map &map)
{
	map(0x0000, 0x0000).w(FUNC(hotblock_state::port0_w));
	map(0x0004, 0x0004).rw(FUNC(hotblock_state::port4_r), FUNC(hotblock_state::port4_w));
	map(0x8000, 0x8001).w("aysnd", FUNC(ym2149_device::address_data_w));
	map(0x8001, 0x8001).r("aysnd", FUNC(ym2149_device::data_r));
}



void hotblock_state::video_start()
{
	save_item(NAME(m_pal));
	save_item(NAME(m_port0));
	save_item(NAME(m_port4)); //stored but not read for now
}

uint32_t hotblock_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int y, x, count;
	int i;
	static const int xxx = 320, yyy = 204;

	bitmap.fill(m_palette->black_pen());

	for (i = 0; i < 256; i++)
	{
		int dat = (m_pal[i * 2 + 1] << 8) | m_pal[i * 2];
		m_palette->set_pen_color(i, pal5bit(dat >> 0), pal5bit(dat >> 5), pal5bit(dat >> 10));
	}

	count = 0;
	for (y = 0; y < yyy; y++)
	{
		for(x = 0; x < xxx; x++)
		{
			if (m_port0 & 0x40)
				bitmap.pix16(y, x) = m_vram[count];
			count++;
		}
	}

	return 0;
}


static INPUT_PORTS_START( hotblock )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) // unused?

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) // used to get test mode
INPUT_PORTS_END


MACHINE_CONFIG_START(hotblock_state::hotblock)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", I8088, 10000000)
	MCFG_DEVICE_PROGRAM_MAP(hotblock_map)
	MCFG_DEVICE_IO_MAP(hotblock_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(8000000, 512, 0, 320, 312, 0, 200) // 15.625 kHz horizontal???
	MCFG_SCREEN_UPDATE_DRIVER(hotblock_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_VBLANK_CALLBACK(INPUTLINE("maincpu", INPUT_LINE_NMI)) // right?

	MCFG_PALETTE_ADD("palette", 256)


	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	MCFG_DEVICE_ADD("aysnd", YM2149, 1000000)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("P1"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("P2"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

ROM_START( hotblock )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "hotblk5.ic4", 0x000000, 0x080000, CRC(5f90f776) SHA1(5ca74714a7d264b4fafaad07dc11e57308828d30) )
	ROM_LOAD( "hotblk6.ic5", 0x080000, 0x080000, CRC(3176d231) SHA1(ac22fd0e9820c6714f51a3d8315eb5d43ef91eeb) )
ROM_END

GAME( 1993, hotblock, 0, hotblock, hotblock, hotblock_state, empty_init, ROT0, "NIX?", "Hot Blocks - Tetrix II", MACHINE_SUPPORTS_SAVE )
