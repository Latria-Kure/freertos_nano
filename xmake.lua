toolchain("arm-none-eabi")
    set_kind("standalone")
    set_sdkdir("D:\\ArmGNUToolchain")
toolchain_end()



local target_name = "stm32_freertos_hello"
target(target_name)
    set_kind("binary")
    set_toolchains("arm-none-eabi")

    add_files(
        "./startup_stm32f407xx.s",
        "Core/Src/*.c",
        "Drivers/STM32F4xx_HAL_Driver/Src/*.c",
        "components/utils/*.c",
        "components/easylogger/**.c|plugins/**.c",
        "components/freertos_nano/**.c"
    )
    remove_files(-- remove hal template file
        "Drivers/STM32F4xx_HAL_Driver/Src/*_template.c"
    )

    add_includedirs(
        "Core/Inc",
        "Drivers/CMSIS/Include",
        "Drivers/STM32F4xx_HAL_Driver/Inc",
        "Drivers/CMSIS/Device/ST/STM32F4xx/Include",
        "components",
        "components/easylogger/inc",
        "components/freertos_nano/inc",
        "components/freertos_nano/portable/GCC/ARM_CM4"
    )


    add_defines(
        "STM32F407xx",
        "USE_HAL_DRIVER"
        -- "USE_FULL_LL_DRIVER", -- enable low level driver
        -- "HSE_STARTUP_TIMEOUT=100",
        -- "LSE_STARTUP_TIMEOUT=5000",
        -- "LSE_VALUE=32768",
        -- "EXTERNAL_CLOCK_VALUE=12288000",
        -- "HSI_VALUE=16000000",
        -- "LSI_VALUE=32000",
        -- "VDD_VALUE=3300",
        -- "PREFETCH_ENABLE=1",
        -- "INSTRUCTION_CACHE_ENABLE=1",
        -- "DATA_CACHE_ENABLE=1"
    )

    add_cflags(
        "-mcpu=cortex-m4",
        "-mthumb",
        "-mfloat-abi=hard",
        "-mfpu=fpv4-sp-d16",
        "-Wall",
        "-fdata-sections",
        "-ffunction-sections",
        "-fno-common",
        "-fmessage-length=0",{force = true}
    )

    add_asflags(
        "-x assembler-with-cpp",
        "-mcpu=cortex-m4",
        "-mthumb",
        "-mfloat-abi=hard",
        "-mfpu=fpv4-sp-d16",
        "-Wall",
        "-fdata-sections",
        "-ffunction-sections",
        "-fno-common",
        "-fmessage-length=0",{force = true}
    )


    add_ldflags(
        "-mcpu=cortex-m4",
        "-mthumb",
        "-mfloat-abi=hard",
        "-mfpu=fpv4-sp-d16",
        "-TSTM32F407VETx_FLASH.ld",
        '-Wl,-Map=$(buildir)/'..target_name..'.map,--cref',
        "-Wl,--gc-sections,--print-memory-usage",
        "-lc -lm -lgcc",
        "--specs=nosys.specs --specs=nano.specs",{force = true}
    )

    if is_mode("release") then
        add_cflags("-Ofast",{force = true})
        add_asflags("-Ofast",{force = true})
        add_ldflags("-Ofast",{force = true})
        add_defines("NDEBUG")
    end

    if is_mode("debug") then
        add_cflags("-Og","-g","-gdwarf-2",{force = true})
        add_asflags("-Og","-g","-gdwarf-2",{force = true})
        add_ldflags("-Og","-g","-gdwarf-2",{force = true})
    end
    if is_mode("minsizerel") then 
        add_cflags("-Os",{force = true})
        add_asflags("-Os",{force = true})
    end
    set_targetdir("build")
    local output_file_name = target_name..".elf"
    set_filename(output_file_name)
    after_build(function(target)
        print("Generating hex and bin file...")
        os.exec('arm-none-eabi-objcopy -O ihex ./build//'..output_file_name..' ./build//'..target_name..'.hex')
        os.exec('arm-none-eabi-objcopy -O binary ./build//'..output_file_name..' ./build//'..target_name..'.bin')
        print("Generate hex and bin file done!")
        print("********************储存空间占用情况*****************************")
        os.exec('arm-none-eabi-size -Ax ./build/'..output_file_name)
        os.exec('arm-none-eabi-size -Bd ./build/'..output_file_name)
    end)

target_end()

