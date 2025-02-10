# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/ndt0012/esp/v5.3.1/esp-idf/components/bootloader/subproject"
  "/home/ndt0012/Documents/ESP32/LCD_ST7920/build/bootloader"
  "/home/ndt0012/Documents/ESP32/LCD_ST7920/build/bootloader-prefix"
  "/home/ndt0012/Documents/ESP32/LCD_ST7920/build/bootloader-prefix/tmp"
  "/home/ndt0012/Documents/ESP32/LCD_ST7920/build/bootloader-prefix/src/bootloader-stamp"
  "/home/ndt0012/Documents/ESP32/LCD_ST7920/build/bootloader-prefix/src"
  "/home/ndt0012/Documents/ESP32/LCD_ST7920/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/ndt0012/Documents/ESP32/LCD_ST7920/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/ndt0012/Documents/ESP32/LCD_ST7920/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
