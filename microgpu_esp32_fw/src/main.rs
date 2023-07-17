use embedded_hal::spi::MODE_3;
use display_interface_spi::SPIInterfaceNoCS;
use esp_idf_hal::delay::Ets;
use esp_idf_hal::gpio::PinDriver;
use esp_idf_hal::spi::*;
use esp_idf_hal::units::FromValueType;
use esp_idf_sys as _;
use ili9341::{DisplaySize240x320, Ili9341, Orientation};
// If using the `binstart` feature of `esp-idf-sys`, always keep this module imported
use log::*;
use microgpu_common::displays::EmbeddedDisplay;
use microgpu_common::FrameBuffer;

fn main() {
    // It is necessary to call this function once. Otherwise some patches to the runtime
    // implemented by esp-idf-sys might not link properly. See https://github.com/esp-rs/esp-idf-template/issues/71
    esp_idf_sys::link_patches();
    // Bind the log crate to the ESP Logging facilities
    esp_idf_svc::log::EspLogger::initialize_default();

    println!("test");

    let peripherals = esp_idf_hal::peripherals::Peripherals::take().unwrap();
    let spi = peripherals.spi2;
    let sclk = peripherals.pins.gpio14;
    let sda = peripherals.pins.gpio13;
    let sdi = peripherals.pins.gpio12;
    let cs = peripherals.pins.gpio11;
    let dc = PinDriver::output(peripherals.pins.gpio10).unwrap();
    let reset = PinDriver::output(peripherals.pins.gpio9).unwrap();

    println!("test2");
    let spi_config = config::Config::new()
        .baudrate(26.MHz().into())
        .data_mode(MODE_3);

    let device = SpiDeviceDriver::new_single(
        spi,
        sclk,
        sda,
        Some(sdi),
        Some(cs),
        &SpiDriverConfig::new(),
        &spi_config,
    ).unwrap();

    println!("test3");
    let di = SPIInterfaceNoCS::new(device, dc);
    let mut lcd = Ili9341::new(
        di,
        reset,
        &mut Ets,
        Orientation::Portrait,
        DisplaySize240x320,
    );

    if lcd.is_err() {
        println!("Error with lcd");
    }  else {
        println!("lcd ok");
    }

    let mut lcd = lcd.unwrap();

    println!("test4");
    let mut frame_buffer = FrameBuffer::new(240, 320);
    for pixel in frame_buffer.mut_pixels() {
        *pixel = 0xFFFF;
    }

    lcd.draw_frame(frame_buffer);

    info!("Hello, world!");

    loop { }
}
