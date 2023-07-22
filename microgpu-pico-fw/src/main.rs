//! # Pico Blinky Example
//!
//! Blinks the LED on a Pico board.
//!
//! This will blink an LED attached to GP25, which is the pin the Pico uses for
//! the on-board LED.
//!
//! See the `Cargo.toml` file for Copyright and license details.

#![no_std]
#![no_main]

use display_interface_spi::SPIInterface;
use embedded_graphics::pixelcolor::Rgb565;
use embedded_graphics::prelude::*;
// The macro for our start-up function
use rp_pico::entry;

// GPIO traits
use embedded_hal::digital::v2::{OutputPin, PinState};
use fugit::RateExtU32;
use mipidsi::Builder;

// Ensure we halt the program on panic (if we don't mention this crate it won't
// be linked)
use panic_halt as _;

// Pull in any important traits
use rp_pico::hal::prelude::*;

// A shorter alias for the Peripheral Access Crate, which provides low-level
// register access
use rp_pico::hal::{gpio, pac, spi};

// A shorter alias for the Hardware Abstraction Layer, which provides
// higher-level drivers.
use rp_pico::hal;

/// Entry point to our bare-metal application.
///
/// The `#[entry]` macro ensures the Cortex-M start-up code calls this function
/// as soon as all global variables are initialised.
///
/// The function configures the RP2040 peripherals, then blinks the LED in an
/// infinite loop.
#[entry]
fn main() -> ! {
    // Grab our singleton objects
    let mut pac = pac::Peripherals::take().unwrap();
    let core = pac::CorePeripherals::take().unwrap();

    // Set up the watchdog driver - needed by the clock setup code
    let mut watchdog = hal::Watchdog::new(pac.WATCHDOG);

    // Configure the clocks
    //
    // The default is to generate a 125 MHz system clock
    let clocks = hal::clocks::init_clocks_and_plls(
        rp_pico::XOSC_CRYSTAL_FREQ,
        pac.XOSC,
        pac.CLOCKS,
        pac.PLL_SYS,
        pac.PLL_USB,
        &mut pac.RESETS,
        &mut watchdog,
    )
    .ok()
    .unwrap();

    let mut delay = cortex_m::delay::Delay::new(core.SYST, clocks.system_clock.freq().to_Hz());

    // The single-cycle I/O block controls our GPIO pins
    let sio = hal::Sio::new(pac.SIO);

    // Set the pins up according to their function on this particular board
    let pins = rp_pico::Pins::new(
        pac.IO_BANK0,
        pac.PADS_BANK0,
        sio.gpio_bank0,
        &mut pac.RESETS,
    );

    let mut pico_led = pins.led.into_push_pull_output();
    let cs = pins.gpio17.into_push_pull_output();
    let reset = pins.gpio10.into_push_pull_output();
    let dc = pins.gpio11.into_push_pull_output();
    let _copi = pins.gpio15.into_mode::<gpio::FunctionSpi>();
    let _sclk = pins.gpio14.into_mode::<gpio::FunctionSpi>();
    let _display_led = pins.gpio12.into_push_pull_output_in_state(PinState::High);

    let spi = spi::Spi::<_, _, 8>::new(pac.SPI1);
    let spi = spi.init(
        &mut pac.RESETS,
        clocks.peripheral_clock.freq(),
        62.MHz(),
        &embedded_hal::spi::MODE_3,
    );

    let di = SPIInterface::new(spi, dc, cs);
    let mut display = Builder::ili9341_rgb565(di)
        .init(&mut delay, Some(reset))
        .unwrap();

    display.clear(Rgb565::RED).unwrap();

    pico_led.set_high().unwrap();


    loop {}
}
