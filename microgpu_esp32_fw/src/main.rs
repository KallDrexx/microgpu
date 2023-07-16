use embedded_hal::spi::MODE_3;
use esp_idf_hal::spi::*;
use esp_idf_hal::units::FromValueType;
use esp_idf_sys as _; // If using the `binstart` feature of `esp-idf-sys`, always keep this module imported
use log::*;

fn main() {
    // It is necessary to call this function once. Otherwise some patches to the runtime
    // implemented by esp-idf-sys might not link properly. See https://github.com/esp-rs/esp-idf-template/issues/71
    esp_idf_sys::link_patches();
    // Bind the log crate to the ESP Logging facilities
    esp_idf_svc::log::EspLogger::initialize_default();

    let peripherals = esp_idf_hal::peripherals::Peripherals::take().unwrap();
    let spi = peripherals.spi2;
    let sclk = peripherals.pins.gpio14;
    let mosi = peripherals.pins.gpio13;
    let miso = peripherals.pins.gpio12;
    let cs = peripherals.pins.gpio11;

    let spi_config = config::Config::new()
        .baudrate(26.MHz().into())
        .data_mode(MODE_3);

    info!("Hello, world!");
}
