cmake_minimum_required(VERSION 3.10)

# Define the geargfx static library
project(geargfx)

set( vendor_dir .. )

# Sources (taken from CMakeVendor.txt)
set( geargfx_src
    ${vendor_dir}/geargfx/adpcm.cpp
    ${vendor_dir}/geargfx/arcade_card_mapper.cpp
    ${vendor_dir}/geargfx/audio.cpp
    ${vendor_dir}/geargfx/cdrom.cpp
    ${vendor_dir}/geargfx/cdrom_audio.cpp
    ${vendor_dir}/geargfx/cdrom_chd_image.cpp
    ${vendor_dir}/geargfx/cdrom_cuebin_image.cpp
    ${vendor_dir}/geargfx/cdrom_image.cpp
    ${vendor_dir}/geargfx/cdrom_media.cpp
    ${vendor_dir}/geargfx/geargrafx_core.cpp
    ${vendor_dir}/geargfx/huc6202.cpp
    ${vendor_dir}/geargfx/huc6260.cpp
    ${vendor_dir}/geargfx/huc6270.cpp
    ${vendor_dir}/geargfx/huc6280.cpp
    ${vendor_dir}/geargfx/huc6280_functors.cpp
    ${vendor_dir}/geargfx/huc6280_opcodes.cpp
    ${vendor_dir}/geargfx/huc6280_psg.cpp
    ${vendor_dir}/geargfx/input.cpp
    ${vendor_dir}/geargfx/mapper.cpp
    ${vendor_dir}/geargfx/media.cpp
    ${vendor_dir}/geargfx/memory.cpp
    ${vendor_dir}/geargfx/scsi_controller.cpp
    ${vendor_dir}/geargfx/sf2_mapper.cpp
    ${vendor_dir}/geargfx/shared/dependencies/miniz/miniz.c
)

# Create static library
add_library(geargfx STATIC ${geargfx_src})

# Public include directories
target_include_directories(geargfx PUBLIC
    ${vendor_dir}/geargfx
    ${vendor_dir}/geargfx/shared/dependencies/miniz
)
