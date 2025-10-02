cmake_minimum_required(VERSION 3.10)

# Define the geargfx static library
project(geargfx)

set( vendor_dir .. )

# Sources (taken from CMakeVendor.txt)
set( geargfx_src
    ${vendor_dir}/geargfx/adpcm.cpp
    ${vendor_dir}/geargfx/adpcm.h
    ${vendor_dir}/geargfx/adpcm_inline.h
    ${vendor_dir}/geargfx/arcade_card_mapper.cpp
    ${vendor_dir}/geargfx/arcade_card_mapper.h
    ${vendor_dir}/geargfx/arcade_card_mapper_inline.h
    ${vendor_dir}/geargfx/audio.cpp
    ${vendor_dir}/geargfx/audio.h
    ${vendor_dir}/geargfx/audio_inline.h
    ${vendor_dir}/geargfx/bit_ops.h
    ${vendor_dir}/geargfx/cdrom.cpp
    ${vendor_dir}/geargfx/cdrom.h
    ${vendor_dir}/geargfx/cdrom_audio.cpp
    ${vendor_dir}/geargfx/cdrom_audio.h
    ${vendor_dir}/geargfx/cdrom_audio_inline.h
    ${vendor_dir}/geargfx/cdrom_chd_image.cpp
    ${vendor_dir}/geargfx/cdrom_chd_image.h
    ${vendor_dir}/geargfx/cdrom_common.h
    ${vendor_dir}/geargfx/cdrom_cuebin_image.cpp
    ${vendor_dir}/geargfx/cdrom_cuebin_image.h
    ${vendor_dir}/geargfx/cdrom_image.cpp
    ${vendor_dir}/geargfx/cdrom_image.h
    ${vendor_dir}/geargfx/cdrom_inline.h
    ${vendor_dir}/geargfx/cdrom_media.cpp
    ${vendor_dir}/geargfx/cdrom_media.h
    ${vendor_dir}/geargfx/cdrom_media_inline.h
    ${vendor_dir}/geargfx/common.h
    ${vendor_dir}/geargfx/crc.h
    ${vendor_dir}/geargfx/defines.h
    ${vendor_dir}/geargfx/game_db.h
    ${vendor_dir}/geargfx/geargrafx.h
    ${vendor_dir}/geargfx/geargrafx_core.cpp
    ${vendor_dir}/geargfx/geargrafx_core.h
    ${vendor_dir}/geargfx/geargrafx_core_inline.h
    ${vendor_dir}/geargfx/huc6202.cpp
    ${vendor_dir}/geargfx/huc6202.h
    ${vendor_dir}/geargfx/huc6202_inline.h
    ${vendor_dir}/geargfx/huc6260.cpp
    ${vendor_dir}/geargfx/huc6260.h
    ${vendor_dir}/geargfx/huc6260_inline.h
    ${vendor_dir}/geargfx/huc6270.cpp
    ${vendor_dir}/geargfx/huc6270.h
    ${vendor_dir}/geargfx/huc6270_defines.h
    ${vendor_dir}/geargfx/huc6270_inline.h
    ${vendor_dir}/geargfx/huc6280.cpp
    ${vendor_dir}/geargfx/huc6280.h
    ${vendor_dir}/geargfx/huc6280_functors.cpp
    ${vendor_dir}/geargfx/huc6280_inline.h
    ${vendor_dir}/geargfx/huc6280_names.h
    ${vendor_dir}/geargfx/huc6280_opcodes.cpp
    ${vendor_dir}/geargfx/huc6280_opcodes_inline.h
    ${vendor_dir}/geargfx/huc6280_psg.cpp
    ${vendor_dir}/geargfx/huc6280_psg.h
    ${vendor_dir}/geargfx/huc6280_psg_inline.h
    ${vendor_dir}/geargfx/huc6280_registers.h
    ${vendor_dir}/geargfx/huc6280_timing.h
    ${vendor_dir}/geargfx/input.cpp
    ${vendor_dir}/geargfx/input.h
    ${vendor_dir}/geargfx/input_inline.h
    ${vendor_dir}/geargfx/log.h
    ${vendor_dir}/geargfx/mapper.cpp
    ${vendor_dir}/geargfx/mapper.h
    ${vendor_dir}/geargfx/media.cpp
    ${vendor_dir}/geargfx/media.h
    ${vendor_dir}/geargfx/media_inline.h
    ${vendor_dir}/geargfx/memory.cpp
    ${vendor_dir}/geargfx/memory.h
    ${vendor_dir}/geargfx/memory_inline.h
    ${vendor_dir}/geargfx/memory_stream.h
    ${vendor_dir}/geargfx/scsi_controller.cpp
    ${vendor_dir}/geargfx/scsi_controller.h
    ${vendor_dir}/geargfx/scsi_controller_inline.h
    ${vendor_dir}/geargfx/sf2_mapper.cpp
    ${vendor_dir}/geargfx/sf2_mapper.h
    ${vendor_dir}/geargfx/types.h
    ${vendor_dir}/geargfx/shared/dependencies/miniz/miniz.c
)

# Create static library
add_library(geargfx STATIC ${geargfx_src})

# Public include directories
target_include_directories(geargfx PUBLIC
    ${vendor_dir}/geargfx
    ${vendor_dir}/geargfx/shared/dependencies/miniz
)
